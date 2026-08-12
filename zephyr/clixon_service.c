/*
 * Copyright (c) 2024 Benjamin Santon ADI
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>

#ifdef CONFIG_CLIXON
/* STM32 compatibility workaround - undef SET/RESET macros from STM32 HAL 
 * before including clixon headers which define SET as an enum value
 */
#ifdef CONFIG_SOC_FAMILY_STM32
#undef SET
#undef RESET  
#endif

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

#ifdef CONFIG_CLIXON_NETCONF_TLS
#include <zephyr/net/tls_credentials.h>
#include <mbedtls/ssl.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/x509_crt.h>
#include <mbedtls/pk.h>
#include <mbedtls/error.h>
#endif /* CONFIG_CLIXON_NETCONF_TLS */
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
#ifdef CONFIG_CLIXON_NETCONF_TLS
mbedtls_ssl_config ssl_conf;
mbedtls_x509_crt server_cert;
mbedtls_pk_context server_key;
mbedtls_entropy_context entropy;
mbedtls_ctr_drbg_context ctr_drbg;
bool tls_initialized;
#endif
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

#ifdef CONFIG_CLIXON_NETCONF_TLS
/* Setup TLS session for this client */
mbedtls_ssl_context ssl;
mbedtls_ssl_init(&ssl);

ret = mbedtls_ssl_setup(&ssl, &clixon_service.ssl_conf);
if (ret != 0) {
LOG_ERR("Failed to setup SSL: -0x%04x", -ret);
close(client_sock);
continue;
}

/* Set the socket for SSL */
mbedtls_ssl_set_bio(&ssl, &client_sock,
(mbedtls_ssl_send_t *)send,
(mbedtls_ssl_recv_t *)recv, NULL);

/* Perform SSL handshake */
LOG_DBG("Starting TLS handshake with client");
while ((ret = mbedtls_ssl_handshake(&ssl)) != 0) {
if (ret != MBEDTLS_ERR_SSL_WANT_READ &&
ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
LOG_ERR("TLS handshake failed: -0x%04x", -ret);
mbedtls_ssl_free(&ssl);
close(client_sock);
goto next_client;
}
}
LOG_INF("TLS handshake successful");

/* Send hello message over TLS */
ret = mbedtls_ssl_write(&ssl, (const unsigned char *)hello_msg,
strlen(hello_msg));
if (ret < 0) {
LOG_ERR("Failed to send hello over TLS: -0x%04x", -ret);
} else {
LOG_INF("Sent NETCONF hello message to client over TLS");
}

mbedtls_ssl_close_notify(&ssl);
mbedtls_ssl_free(&ssl);
#else
send(client_sock, hello_msg, strlen(hello_msg), 0);
LOG_INF("Sent NETCONF hello message to client");
#endif

/* Close client socket for now */
close(client_sock);
LOG_INF("Client connection closed");

#ifdef CONFIG_CLIXON_NETCONF_TLS
next_client:
continue;
#endif
}

close(clixon_service.server_sock);
LOG_INF("NETCONF server thread stopped");
}

#ifdef CONFIG_CLIXON_NETCONF_TLS
/* Embedded test certificates (self-signed for testing only) */
#include "test_certs/server_cert.h"
#include "test_certs/server_key.h"

/**
 * @brief Initialize TLS/SSL context
 */
static int tls_init(void)
{
int ret;
const char *pers = "netconf_server";

LOG_INF("Initializing TLS for NETCONF");

/* Initialize mbedTLS structures */
mbedtls_ssl_config_init(&clixon_service.ssl_conf);
mbedtls_x509_crt_init(&clixon_service.server_cert);
mbedtls_pk_init(&clixon_service.server_key);
mbedtls_entropy_init(&clixon_service.entropy);
mbedtls_ctr_drbg_init(&clixon_service.ctr_drbg);

/* Seed the random number generator */
ret = mbedtls_ctr_drbg_seed(&clixon_service.ctr_drbg,
mbedtls_entropy_func,
&clixon_service.entropy,
(const unsigned char *)pers,
strlen(pers));
if (ret != 0) {
LOG_ERR("Failed to seed RNG: -0x%04x", -ret);
return -EINVAL;
}

/* Load embedded server certificate */
	ret = mbedtls_x509_crt_parse_der(&clixon_service.server_cert,
					  server_cert_der,
					  server_cert_der_len);
	if (ret != 0) {
		LOG_ERR("Failed to parse server certificate: -0x%04x", -ret);
		return -EINVAL;
	}
	LOG_INF("Server certificate loaded (%u bytes)", server_cert_der_len);

	/* Load embedded server private key */
	ret = mbedtls_pk_parse_key(&clixon_service.server_key,
				    server_key_der,
				    server_key_der_len,
				    NULL, 0, /* No password */
				    mbedtls_ctr_drbg_random,
				    &clixon_service.ctr_drbg);
	if (ret != 0) {
		LOG_ERR("Failed to parse server key: -0x%04x", -ret);
		return -EINVAL;
	}
	LOG_INF("Server private key loaded (%u bytes)", server_key_der_len);

/* Setup SSL configuration */
ret = mbedtls_ssl_config_defaults(&clixon_service.ssl_conf,
MBEDTLS_SSL_IS_SERVER,
MBEDTLS_SSL_TRANSPORT_STREAM,
MBEDTLS_SSL_PRESET_DEFAULT);
if (ret != 0) {
LOG_ERR("Failed to set SSL config defaults: -0x%04x", -ret);
return -EINVAL;
}

mbedtls_ssl_conf_rng(&clixon_service.ssl_conf,
mbedtls_ctr_drbg_random,
&clixon_service.ctr_drbg);

/* Configure our certificate */
ret = mbedtls_ssl_conf_own_cert(&clixon_service.ssl_conf,
&clixon_service.server_cert,
&clixon_service.server_key);
if (ret != 0) {
LOG_ERR("Failed to configure server certificate: -0x%04x", -ret);
return -EINVAL;
}

/* Set auth mode to none for testing (no client cert required) */
mbedtls_ssl_conf_authmode(&clixon_service.ssl_conf,
MBEDTLS_SSL_VERIFY_NONE);

clixon_service.tls_initialized = true;
LOG_INF("TLS context initialized with embedded test certificates");
LOG_WRN("Using self-signed test certificates - NOT FOR PRODUCTION");

return 0;
}

/**
 * @brief Cleanup TLS/SSL context
 */
static void tls_cleanup(void)
{
if (clixon_service.tls_initialized) {
mbedtls_ssl_config_free(&clixon_service.ssl_conf);
mbedtls_x509_crt_free(&clixon_service.server_cert);
mbedtls_pk_free(&clixon_service.server_key);
mbedtls_ctr_drbg_free(&clixon_service.ctr_drbg);
mbedtls_entropy_free(&clixon_service.entropy);
clixon_service.tls_initialized = false;
LOG_INF("TLS context cleaned up");
}
}
#endif /* CONFIG_CLIXON_NETCONF_TLS */

/**
 * @brief Initialize NETCONF server
 */
static int netconf_init(void)
{
int ret;

LOG_INF("Starting NETCONF server on port %d", CONFIG_CLIXON_NETCONF_PORT);

#ifdef CONFIG_CLIXON_NETCONF_TLS
/* Initialize TLS if enabled */
ret = tls_init();
if (ret < 0) {
LOG_ERR("Failed to initialize TLS");
return ret;
}
LOG_INF("NETCONF server will use TLS encryption");
#else
LOG_WRN("NETCONF server running WITHOUT TLS encryption");
#endif

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

#ifdef CONFIG_CLIXON_NETCONF_TLS
/* Cleanup TLS context */
tls_cleanup();
#endif
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
