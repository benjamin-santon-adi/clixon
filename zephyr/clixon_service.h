/*
 * Copyright (c) 2024 Benjamin Santon ADI
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLIXON_SERVICE_H_
#define CLIXON_SERVICE_H_

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Check if Clixon service is initialized
 *
 * @return true if service is initialized, false otherwise
 */
bool clixon_service_is_initialized(void);

#ifdef __cplusplus
}
#endif

#endif /* CLIXON_SERVICE_H_ */
