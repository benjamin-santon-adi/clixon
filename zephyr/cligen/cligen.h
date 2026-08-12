/*
 * Copyright (c) 2024 Benjamin Santon ADI
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * CLIgen compatibility stub for Zephyr
 * 
 * This is a minimal stub to allow clixon core library to compile.
 * Full CLIgen integration is future work.
 * For now, any functions requiring CLIgen will return errors.
 */

#ifndef CLIXON_ZEPHYR_CLIGEN_H_
#define CLIXON_ZEPHYR_CLIGEN_H_

#include <stdint.h>
#include <stdbool.h>

/* Stub types for CLIgen structures */
typedef struct cligen_handle cligen_handle;
typedef struct cvec cvec;
typedef struct cg_var cg_var;
typedef struct cg_obj cg_obj;
typedef struct parse_tree parse_tree;

/* Stub defines */
#define CGV_STRING 1
#define CGV_INT    2

/* Stub function declarations - these will need proper implementation */
/* For now they are declared but not defined, or will return NULL/errors */

#ifdef __cplusplus
extern "C" {
#endif

/* Minimal stubs that return NULL or error codes */
static inline cligen_handle *cligen_init(void) { return NULL; }
static inline int cligen_exit(cligen_handle *h) { return -1; }
static inline cvec *cvec_new(int len) { return NULL; }
static inline int cvec_free(cvec *cv) { return -1; }

/* Add more stubs as needed during compilation */

#ifdef __cplusplus
}
#endif

#endif /* CLIXON_ZEPHYR_CLIGEN_H_ */
