/*
 * Copyright (c) 2024 Benjamin Santon ADI
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(clixon_service, CONFIG_CLIXON_LOG_LEVEL);

/* Clixon service state */
static struct {
	bool initialized;
	/* Add clixon handle and other state here when library is ported */
	/* clixon_handle handle; */
} clixon_service;

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
	LOG_INF("Initializing Clixon service");

	if (clixon_service.initialized) {
		LOG_WRN("Clixon service already initialized");
		return 0;
	}

	/* TODO: Initialize Clixon library here when ported to Zephyr */
	/* Example:
	 * clixon_service.handle = clixon_handle_init();
	 * if (clixon_service.handle == NULL) {
	 *     LOG_ERR("Failed to initialize Clixon handle");
	 *     return -EINVAL;
	 * }
	 */

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

/* Initialize the service at APPLICATION level, before main() */
SYS_INIT(clixon_service_init, APPLICATION, CONFIG_CLIXON_SERVICE_INIT_PRIORITY);
