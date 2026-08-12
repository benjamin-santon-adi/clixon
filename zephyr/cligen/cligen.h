/*
 * Copyright (c) 2024 Benjamin Santon ADI
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * CLIgen minimal compatibility for Zephyr  
 * 
 * This provides minimal stubs/types to allow clixon core library to compile.
 * Full CLIgen functionality is not implemented - this is for non-CLI embedded use.
 * 
 * Based on CLIgen API from github.com/clicon/cligen
 */

#ifndef CLIXON_ZEPHYR_CLIGEN_H_
#define CLIXON_ZEPHYR_CLIGEN_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations for opaque types */
typedef struct cligen_handle cligen_handle;
typedef struct parse_tree parse_tree;
typedef struct cg_obj cg_obj;

/* Callback function types */
typedef int (*cligen_susp_cb_t)(cligen_handle h, char *fn);
typedef int (*cligen_interrupt_cb_t)(cligen_handle h);
typedef int (*cligen_fd_cb_t)(int fd, void *arg);

/* CVec - CLIgen vector of variables */
typedef struct cvec {
    struct cg_var **vr_vec;   /* vector of pointers to cg_var */
    int             vr_len;   /* length of vector */
} cvec;

/* CG variable types */
enum cv_type {
    CGV_ERR = -1,      /* ERROR */
    CGV_INT8,          /* 8-bit signed integer */
    CGV_INT16,         /* 16-bit signed integer */
    CGV_INT32,         /* 32-bit signed integer */
    CGV_INT64,         /* 64-bit signed integer */
    CGV_UINT8,         /* 8-bit unsigned integer */
    CGV_UINT16,        /* 16-bit unsigned integer */
    CGV_UINT32,        /* 32-bit unsigned integer */
    CGV_UINT64,        /* 64-bit unsigned integer */
    CGV_DEC64,         /* decimal64 */
    CGV_BOOL,          /* boolean */
    CGV_REST,          /* rest */
    CGV_STRING,        /* string */
    CGV_INTERFACE,     /* interface */
    CGV_IPV4ADDR,      /* IPv4 address */
    CGV_IPV4PFX,       /* IPv4 prefix */
    CGV_IPV6ADDR,      /* IPv6 address */
    CGV_IPV6PFX,       /* IPv6 prefix */
    CGV_MACADDR,       /* MAC address */
    CGV_URL,           /* URL */
    CGV_UUID,          /* UUID */
    CGV_TIME,          /* Time */
    CGV_VOID,          /* Void */
    CGV_EMPTY,         /* Empty type */
};

/* CG Variable - single variable with type and value */
typedef struct cg_var {
    char          *var_name;     /* name */
    enum cv_type   var_type;     /* type */
    char          *var_show;     /* help text or NULL */
    union {
        int8_t      vari8;
        int16_t     vari16;
        int32_t     vari32;
        int64_t     vari64;
        uint8_t     varu8;
        uint16_t    varu16;
        uint32_t    varu32;
        uint64_t    varu64;
        char       *string;
        void       *ptr;
    } var_val;
    void          *var_userdata; /* user data */
} cg_var;

/* CBuf - CLIgen buffer for string building */
typedef struct {
    char  *cb_buffer;    /* buffer */
    size_t cb_buflen;    /* buffer length */
    size_t cb_strlen;    /* string length (used) */
} cbuf;

/*
 * CG_VAR functions
 */
enum cv_type cv_type_get(cg_var *cv);
char *cv_name_get(cg_var *cv);
int cv_name_set(cg_var *cv, const char *name);
char *cv_string_get(cg_var *cv);
int cv_string_set(cg_var *cv, const char *str);
int64_t cv_int64_get(cg_var *cv);
uint64_t cv_uint64_get(cg_var *cv);
cg_var *cv_new(enum cv_type type);
int cv_free(cg_var *cv);
size_t cv_size(cg_var *cv);

/*
 * CVec functions
 */
cvec *cvec_new(int len);
int cvec_free(cvec *cv);
int cvec_len(cvec *cv);
cg_var *cvec_i(cvec *cv, int i);
cg_var *cvec_add(cvec *cv, enum cv_type type);
cg_var *cvec_each(cvec *cv, cg_var *prev);
int cvec_reset(cvec *cv);
size_t cvec_size(cvec *cv);

/*
 * CBUF functions
 */
cbuf *cbuf_new(void);
int cbuf_free(cbuf *cb);
char *cbuf_get(cbuf *cb);
int cbuf_len(cbuf *cb);
int cbuf_reset(cbuf *cb);
int cbuf_append(cbuf *cb, char c);
int cbuf_append_str(cbuf *cb, const char *str);
int cbuf_append_buf(cbuf *cb, const void *buf, size_t len);

/*
 * CLIgen handle functions - stubs (not used in core library)
 */
static inline cligen_handle *cligen_init(void) { return NULL; }
static inline int cligen_exit(cligen_handle *h) { return -1; }

#ifdef __cplusplus
}
#endif

#endif /* CLIXON_ZEPHYR_CLIGEN_H_ */
