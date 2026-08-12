/*
 * Copyright (c) 2024 Benjamin Santon ADI
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * NETCONF over TLS test application
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_core.h>
#include <zephyr/net/net_context.h>
#include <zephyr/net/net_mgmt.h>

#ifdef CONFIG_CLIXON_SERVICE
#include "clixon_service.h"
#endif

LOG_MODULE_REGISTER(netconf_test, LOG_LEVEL_INF);

#define TEST_BANNER "======================================"

static struct net_mgmt_event_callback mgmt_cb;

static void iface_cb(struct net_mgmt_event_callback *cb,
		     uint64_t mgmt_event, struct net_if *iface)
{
	ARG_UNUSED(cb);
	
	if (mgmt_event == NET_EVENT_IPV4_ADDR_ADD) {
		LOG_INF("IPv4 address added to interface %d",
			net_if_get_by_iface(iface));
	}
}

int main(void)
{
	struct net_if *iface;
	
	LOG_INF("%s", TEST_BANNER);
	LOG_INF("NETCONF over TLS Test");
	LOG_INF("%s", TEST_BANNER);
	
	/* Get the default network interface */
	iface = net_if_get_default();
	if (!iface) {
		LOG_ERR("No network interface found");
		return -1;
	}
	
	LOG_INF("Network interface: %d", net_if_get_by_iface(iface));
	
	/* Register for network events */
	net_mgmt_init_event_callback(&mgmt_cb, iface_cb,
				      NET_EVENT_IPV4_ADDR_ADD);
	net_mgmt_add_event_callback(&mgmt_cb);
	
#ifdef CONFIG_CLIXON_SERVICE
	/* Wait for Clixon service to initialize */
	k_sleep(K_MSEC(500));
	
	if (clixon_service_is_initialized()) {
		LOG_INF("Clixon service initialized successfully");
		LOG_INF("NETCONF server listening on port %d", 
			CONFIG_CLIXON_NETCONF_PORT);
#ifdef CONFIG_CLIXON_NETCONF_TLS
		LOG_INF("TLS encryption: ENABLED");
#else
		LOG_INF("TLS encryption: DISABLED");
#endif
	} else {
		LOG_ERR("Clixon service failed to initialize");
		return -1;
	}
#endif
	
	LOG_INF("%s", TEST_BANNER);
	LOG_INF("Test ready - waiting for client connections");
	LOG_INF("%s", TEST_BANNER);
	
	/* Keep running to serve NETCONF requests */
	while (1) {
		k_sleep(K_SECONDS(1));
	}
	
	return 0;
}
