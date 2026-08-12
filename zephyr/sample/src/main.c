/*
 * Copyright (c) 2024 Benjamin Santon ADI
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

/* Clixon headers would be included here when ready */
/* #include <clixon/clixon.h> */

#ifdef CONFIG_CLIXON_SERVICE
#include "clixon_service.h"
#endif

int main(void)
{
	printk("Clixon Sample Application\n");
	printk("=========================\n\n");

	printk("Clixon version: %s\n", "0.1.0");
	printk("This is a sample application demonstrating Clixon library integration\n");
	printk("with Zephyr RTOS.\n\n");

#ifdef CONFIG_CLIXON_SERVICE
	/* Check if Clixon service was initialized via SYS_INIT */
	if (clixon_service_is_initialized()) {
		printk("Clixon service was initialized before main() via SYS_INIT\n");
	} else {
		printk("Clixon service is not initialized\n");
	}
#else
	/* Initialize Clixon library manually */
	printk("Initializing Clixon...\n");
	
	/* TODO: Add Clixon initialization code here */
	/* Example:
	 * clixon_handle h = clixon_handle_init();
	 * if (h == NULL) {
	 *     printk("Failed to initialize Clixon\n");
	 *     return -1;
	 * }
	 */

	printk("Clixon initialization complete\n");
#endif

	/* Example usage of Clixon features */
	printk("\nDemonstrating Clixon features:\n");
	printk("- YANG-based configuration management\n");
	printk("- NETCONF/RESTCONF interfaces\n");
	printk("- Embedded database\n");
	printk("- Transaction mechanism\n");

	/* Main loop */
	while (1) {
		k_sleep(K_SECONDS(5));
		printk("Clixon sample running...\n");
		
		/* Add periodic Clixon operations here */
	}

	return 0;
}
