/*
 * Copyright (c) 2024 Benjamin Santon ADI
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>

#ifdef CONFIG_CLIXON
#ifdef HAVE_CONFIG_H
#include "clixon_config.h"
#endif

#include <cligen/cligen.h>
#include "clixon_queue.h"
#include "clixon_hash.h"
#include "clixon_handle.h"
#include "clixon_yang.h"
#include "clixon_xml.h"
#include "clixon_err.h"
#include "clixon_log.h"
#include "clixon_debug.h"
#include "clixon_options.h"
#endif /* CONFIG_CLIXON */

#ifdef CONFIG_CLIXON_NETCONF
#include <zephyr/net/socket.h>
#include <zephyr/net/net_ip.h>
#include <errno.h>
#include "clixon_netconf_lib.h"
#endif /* CONFIG_CLIXON_NETCONF */

LOG_MODULE_REGISTER(clixon_service, CONFIG_CLIXON_LOG_LEVEL);

/* Clixon service state */
static struct {
bool initialized;
clixon_handle handle;
#ifdef CONFIG_CLIXON_NETCONF
int server_sock;
k_tid_t netconf_thread_id;
bool netconf_running;
#endif
} clixon_service;

#ifdef CONFIG_CLIXON_NETCONF

#define NETCONF_THREAD_STACK_SIZE 4096
#define NETCONF_THREAD_PRIORITY 7
#define NETCONF_MAX_CLIENTS 4

K_THREAD_STACK_DEFINE(netconf_thread_stack, NETCONF_THREAD_STACK_SIZE);
static struct k_thread netconf_thread_data;

/**
 * @brief NETCONF server thread
 *
 * Listens for incoming NETCONF connections and handles client sessions.
 */
static void netconf_server_thread(void *arg1, void *arg2, void *arg3)
{
ARG_UNUSED(arg1);
ARG_UNUSED(arg2);
ARG_UNUSED(arg3);

struct sockaddr_in bind_addr;
int ret;

LOG_INF("NETCONF server thread started");

/* Create TCP socket */
clixon_service.server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
if (clixon_service.server_sock < 0) {
LOG_ERR("Failed to create socket: %d", errno);
return;
}

/* Enable address reuse */
int optval = 1;
ret = setsockopt(clixon_service.server_sock, SOL_SOCKET, SO_REUSEADDR,
 &optval, sizeof(optval));
if (ret < 0) {
LOG_WRN("Failed to set SO_REUSEADDR: %d", errno);
}

/* Bind to NETCONF port */
memset(&bind_addr, 0, sizeof(bind_addr));
bind_addr.sin_family = AF_INET;
bind_addr.sin_addr.s_addr = INADDR_ANY;
bind_addr.sin_port = htons(CONFIG_CLIXON_NETCONF_PORT);

ret = bind(clixon_service.server_sock, (struct sockaddr *)&bind_addr,
   sizeof(bind_addr));
if (ret < 0) {
LOG_ERR("Failed to bind to port %d: %d", 
CONFIG_CLIXON_NETCONF_PORT, errno);
close(clixon_service.server_sock);
return;
}

/* Listen for connections */
ret = listen(clixon_service.server_sock, NETCONF_MAX_CLIENTS);
if (ret < 0) {
LOG_ERR("Failed to listen: %d", errno);
close(clixon_service.server_sock);
return;
}

LOG_INF("NETCONF server listening on port %d", CONFIG_CLIXON_NETCONF_PORT);
clixon_service.netconf_running = true;

/* Accept loop */
while (clixon_service.netconf_running) {
struct sockaddr_in client_addr;
socklen_t client_addr_len = sizeof(client_addr);
int client_sock;

LOG_DBG("Waiting for NETCONF client connection...");

client_sock = accept(clixon_service.server_sock,
     (struct sockaddr *)&client_addr,
     &client_addr_len);

if (client_sock < 0) {
if (errno == EINTR || !clixon_service.netconf_running) {
/* Interrupted or shutting down */
break;
}
LOG_ERR("Accept failed: %d", errno);
k_sleep(K_MSEC(100));
continue;
}

char client_ip[INET_ADDRSTRLEN];
inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
LOG_INF("NETCONF client connected from %s:%d",
client_ip, ntohs(client_addr.sin_port));

/* TODO: Handle client session in separate thread or async
 * For now, just send hello and close
 * Future: netconf_handle_session(client_sock);
 */

const char *hello_msg = 
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<hello xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">\n"
"  <capabilities>\n"
"    <capability>urn:ietf:params:netconf:base:1.0</capability>\n"
"    <capability>urn:ietf:params:netconf:base:1.1</capability>\n"
"  </capabilities>\n"
"  <session-id>1</session-id>\n"
"</hello>\n"
"]]>]]>\n";

send(client_sock, hello_msg, strlen(hello_msg), 0);
LOG_INF("Sent NETCONF hello message to client");

/* Close client socket for now */
close(client_sock);
LOG_INF("Client connection closed");
}

close(clixon_service.server_sock);
LOG_INF("NETCONF server thread stopped");
}

/**
 * @brief Initialize NETCONF server
 */
static int netconf_init(void)
{
LOG_INF("Starting NETCONF server on port %d", CONFIG_CLIXON_NETCONF_PORT);

clixon_service.netconf_running = false;

/* Create NETCONF server thread */
clixon_service.netconf_thread_id = k_thread_create(
&netconf_thread_data,
netconf_thread_stack,
K_THREAD_STACK_SIZEOF(netconf_thread_stack),
netconf_server_thread,
NULL, NULL, NULL,
NETCONF_THREAD_PRIORITY, 0, K_NO_WAIT);

if (clixon_service.netconf_thread_id == NULL) {
LOG_ERR("Failed to create NETCONF server thread");
return -ENOMEM;
}

k_thread_name_set(clixon_service.netconf_thread_id, "netconf_srv");
LOG_INF("NETCONF server initialized");

return 0;
}

/**
 * @brief Shutdown NETCONF server
 */
static void netconf_shutdown(void)
{
if (clixon_service.netconf_running) {
LOG_INF("Shutting down NETCONF server");
clixon_service.netconf_running = false;

/* Close server socket to unblock accept() */
if (clixon_service.server_sock >= 0) {
close(clixon_service.server_sock);
}

/* Wait for thread to exit */
k_thread_join(clixon_service.netconf_thread_id, K_SECONDS(5));
}
}

#endif /* CONFIG_CLIXON_NETCONF */

/**
 * @brief Initialize the Clixon service
 *
 * This function is called automatically during system initialization
 * before main() when CONFIG_CLIXON_SERVICE is enabled.
 *
 * @return 0 on success, negative errno on failure
 */
static int clixon_service_init(void)
{
int ret;

LOG_INF("Initializing Clixon service");

if (clixon_service.initialized) {
LOG_WRN("Clixon service already initialized");
return 0;
}

/* Initialize Clixon library */
clixon_service.handle = clixon_handle_init();
if (clixon_service.handle == NULL) {
LOG_ERR("Failed to initialize Clixon handle");
return -EINVAL;
}

/* Initialize error handling */
ret = clixon_err_init(clixon_service.handle);
if (ret < 0) {
LOG_ERR("Failed to initialize error handling");
return -EINVAL;
}

/* Initialize logging (use numeric value 6 for LOG_INFO to avoid conflict) */
ret = clixon_log_init(clixon_service.handle, "clixon", 6, CLIXON_LOG_SYSLOG);
if (ret < 0) {
LOG_ERR("Failed to initialize logging");
return -EINVAL;
}

/* Initialize debug */
ret = clixon_debug_init(clixon_service.handle, 0);
if (ret < 0) {
LOG_ERR("Failed to initialize debug");
return -EINVAL;
}

/* Initialize options */
ret = clicon_options_main(clixon_service.handle);
if (ret < 0) {
LOG_ERR("Failed to initialize options");
return -EINVAL;
}

#ifdef CONFIG_CLIXON_NETCONF
/* Initialize NETCONF server */
ret = netconf_init();
if (ret < 0) {
LOG_ERR("Failed to initialize NETCONF server");
return ret;
}
#endif

clixon_service.initialized = true;
LOG_INF("Clixon service initialized successfully");

return 0;
}

/**
 * @brief Get Clixon service initialization status
 *
 * @return true if service is initialized, false otherwise
 */
bool clixon_service_is_initialized(void)
{
return clixon_service.initialized;
}

/**
 * @brief Get the Clixon handle from the service
 *
 * @return clixon_handle or NULL if not initialized
 */
clixon_handle clixon_service_get_handle(void)
{
return clixon_service.handle;
}

/* Initialize the service at APPLICATION level, before main() */
SYS_INIT(clixon_service_init, APPLICATION, CONFIG_CLIXON_SERVICE_INIT_PRIORITY);
