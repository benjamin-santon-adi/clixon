/*
 * Copyright (c) 2024 Benjamin Santon ADI
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Clixon Datastore Backend for Zephyr - Public API
 */

#ifndef _CLIXON_DATASTORE_H_
#define _CLIXON_DATASTORE_H_

#ifdef CONFIG_CLIXON
#include "clixon_handle.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize Clixon datastore backend
 * 
 * Creates directory structure and initializes empty datastore files
 * on the configured filesystem (LittleFS, FAT, or NVS).
 * 
 * @param h Clixon handle (can be NULL for standalone initialization)
 * @return 0 on success, negative errno on failure
 */
int clixon_datastore_init(clixon_handle h);

/**
 * @brief Check if datastore is initialized
 * 
 * @return true if initialized, false otherwise
 */
bool clixon_datastore_is_initialized(void);

/**
 * @brief Get datastore directory path
 * 
 * @return Datastore directory path (e.g., "/lfs/clixon")
 */
const char *clixon_datastore_get_dir(void);

#ifdef __cplusplus
}
#endif

#endif /* _CLIXON_DATASTORE_H_ */
