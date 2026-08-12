/*
 * Copyright (c) 2024 Benjamin Santon ADI
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * CLIgen minimal implementation for Zephyr
 */

#include "cligen.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/*
 * CVec functions
 */
cvec *cvec_new(int len)
{
    cvec *cv = (cvec *)calloc(1, sizeof(cvec));
    if (cv && len > 0) {
        cv->vr_vec = (struct cg_var **)calloc(len, sizeof(struct cg_var *));
        if (!cv->vr_vec) {
            free(cv);
            return NULL;
        }
        cv->vr_len = len;
    }
    return cv;
}

int cvec_free(cvec *cv)
{
    int i;
    if (cv) {
        if (cv->vr_vec) {
            for (i = 0; i < cv->vr_len; i++) {
                if (cv->vr_vec[i])
                    cv_free(cv->vr_vec[i]);
            }
            free(cv->vr_vec);
        }
        free(cv);
    }
    return 0;
}

int cvec_len(cvec *cv)
{
    return cv ? cv->vr_len : 0;
}

cg_var *cvec_i(cvec *cv, int i)
{
    if (cv && i >= 0 && i < cv->vr_len)
        return cv->vr_vec[i];
    return NULL;
}

cg_var *cvec_add(cvec *cv, enum cv_type type)
{
    cg_var *cgv;
    cg_var **new_vec;
    int new_len;

    if (!cv)
        return NULL;

    cgv = cv_new(type);
    if (!cgv)
        return NULL;

    /* Expand vector */
    new_len = cv->vr_len + 1;
    new_vec = (cg_var **)realloc(cv->vr_vec, new_len * sizeof(cg_var *));
    if (!new_vec) {
        cv_free(cgv);
        return NULL;
    }

    cv->vr_vec = new_vec;
    cv->vr_vec[cv->vr_len] = cgv;
    cv->vr_len = new_len;

    return cgv;
}

cg_var *cvec_each(cvec *cv, cg_var *prev)
{
    int i;

    if (!cv || !cv->vr_vec)
        return NULL;

    if (!prev) {
        /* Return first */
        return cv->vr_len > 0 ? cv->vr_vec[0] : NULL;
    }

    /* Find prev and return next */
    for (i = 0; i < cv->vr_len - 1; i++) {
        if (cv->vr_vec[i] == prev)
            return cv->vr_vec[i + 1];
    }

    return NULL;
}

int cvec_reset(cvec *cv)
{
    int i;

    if (!cv)
        return -1;

    if (cv->vr_vec) {
        for (i = 0; i < cv->vr_len; i++) {
            if (cv->vr_vec[i]) {
                cv_free(cv->vr_vec[i]);
                cv->vr_vec[i] = NULL;
            }
        }
    }
    cv->vr_len = 0;

    return 0;
}

/*
 * CG_VAR functions
 */
enum cv_type cv_type_get(cg_var *cv)
{
    return cv ? cv->var_type : CGV_ERR;
}

char *cv_name_get(cg_var *cv)
{
    return cv ? cv->var_name : NULL;
}

int cv_name_set(cg_var *cv, const char *name)
{
    if (!cv)
        return -1;

    if (cv->var_name)
        free(cv->var_name);

    cv->var_name = name ? strdup(name) : NULL;
    return 0;
}

char *cv_string_get(cg_var *cv)
{
    return (cv && cv->var_type == CGV_STRING) ? cv->var_val.string : NULL;
}

int cv_string_set(cg_var *cv, const char *str)
{
    if (!cv)
        return -1;

    if (cv->var_type == CGV_STRING && cv->var_val.string)
        free(cv->var_val.string);

    cv->var_type = CGV_STRING;
    cv->var_val.string = str ? strdup(str) : NULL;

    return 0;
}

int64_t cv_int64_get(cg_var *cv)
{
    return cv ? cv->var_val.vari64 : 0;
}

uint64_t cv_uint64_get(cg_var *cv)
{
    return cv ? cv->var_val.varu64 : 0;
}

cg_var *cv_new(enum cv_type type)
{
    cg_var *cv = (cg_var *)calloc(1, sizeof(cg_var));
    if (cv) {
        cv->var_type = type;
    }
    return cv;
}

int cv_free(cg_var *cv)
{
    if (cv) {
        if (cv->var_name)
            free(cv->var_name);
        if (cv->var_show)
            free(cv->var_show);
        if (cv->var_type == CGV_STRING && cv->var_val.string)
            free(cv->var_val.string);
        free(cv);
    }
    return 0;
}

size_t cv_size(cg_var *cv)
{
    size_t sz = sizeof(cg_var);
    
    if (cv) {
        if (cv->var_name)
            sz += strlen(cv->var_name) + 1;
        if (cv->var_show)
            sz += strlen(cv->var_show) + 1;
        if (cv->var_type == CGV_STRING && cv->var_val.string)
            sz += strlen(cv->var_val.string) + 1;
    }
    return sz;
}

size_t cvec_size(cvec *cv)
{
    size_t sz = sizeof(cvec);
    int i;
    
    if (cv) {
        sz += cv->vr_len * sizeof(cg_var *);
        for (i = 0; i < cv->vr_len; i++) {
            if (cv->vr_vec[i])
                sz += cv_size(cv->vr_vec[i]);
        }
    }
    return sz;
}

/*
 * CBUF functions
 */
cbuf *cbuf_new(void)
{
    cbuf *cb = (cbuf *)calloc(1, sizeof(cbuf));
    if (cb) {
        cb->cb_buflen = 1024;
        cb->cb_buffer = (char *)malloc(cb->cb_buflen);
        if (cb->cb_buffer) {
            cb->cb_buffer[0] = '\0';
            cb->cb_strlen = 0;
        } else {
            free(cb);
            cb = NULL;
        }
    }
    return cb;
}

int cbuf_free(cbuf *cb)
{
    if (cb) {
        if (cb->cb_buffer)
            free(cb->cb_buffer);
        free(cb);
    }
    return 0;
}

char *cbuf_get(cbuf *cb)
{
    return cb ? cb->cb_buffer : NULL;
}

int cbuf_len(cbuf *cb)
{
    return cb ? cb->cb_strlen : 0;
}

int cbuf_reset(cbuf *cb)
{
    if (cb && cb->cb_buffer) {
        cb->cb_buffer[0] = '\0';
        cb->cb_strlen = 0;
    }
    return 0;
}

static int cbuf_expand(cbuf *cb, size_t need)
{
    char *new_buf;
    size_t new_len;

    if (!cb || !cb->cb_buffer)
        return -1;

    if (cb->cb_strlen + need + 1 <= cb->cb_buflen)
        return 0; /* Enough space */

    /* Double buffer size or add needed space, whichever is larger */
    new_len = cb->cb_buflen * 2;
    if (new_len < cb->cb_strlen + need + 1)
        new_len = cb->cb_strlen + need + 1;

    new_buf = (char *)realloc(cb->cb_buffer, new_len);
    if (!new_buf)
        return -1;

    cb->cb_buffer = new_buf;
    cb->cb_buflen = new_len;

    return 0;
}

int cbuf_append(cbuf *cb, char c)
{
    if (!cb || !cb->cb_buffer)
        return -1;

    if (cbuf_expand(cb, 1) < 0)
        return -1;

    cb->cb_buffer[cb->cb_strlen++] = c;
    cb->cb_buffer[cb->cb_strlen] = '\0';

    return 0;
}

int cbuf_append_str(cbuf *cb, const char *str)
{
    size_t len;

    if (!cb || !str)
        return -1;

    len = strlen(str);
    if (cbuf_expand(cb, len) < 0)
        return -1;

    memcpy(cb->cb_buffer + cb->cb_strlen, str, len);
    cb->cb_strlen += len;
    cb->cb_buffer[cb->cb_strlen] = '\0';

    return 0;
}

int cbuf_append_buf(cbuf *cb, const void *buf, size_t len)
{
    if (!cb || !buf)
        return -1;

    if (cbuf_expand(cb, len) < 0)
        return -1;

    memcpy(cb->cb_buffer + cb->cb_strlen, buf, len);
    cb->cb_strlen += len;
    cb->cb_buffer[cb->cb_strlen] = '\0';

    return 0;
}
