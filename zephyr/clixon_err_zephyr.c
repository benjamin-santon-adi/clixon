/*
 * Zephyr-specific error handling for Clixon
 * Maps Clixon error system to Zephyr logging
 */

#ifdef HAVE_CONFIG_H
#include "clixon_config.h"
#endif

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

LOG_MODULE_REGISTER(clixon_err, CONFIG_CLIXON_LOG_LEVEL);

#include <cligen/cligen.h>

/* Clixon headers */
#include "clixon_queue.h"
#include "clixon_hash.h"
#include "clixon_handle.h"
#include "clixon_xml.h"
#include "clixon_err.h"

/* Thread-local error state (simplified for embedded) */
static struct {
    int category;
    int subnr;
    char reason[256];
    char str[512];
} err_state = {0};

static const char *category_str(int cat) {
    switch(cat) {
    case OE_DB: return "DB";
    case OE_DAEMON: return "DAEMON";
    case OE_EVENTS: return "EVENTS";
    case OE_CFG: return "CFG";
    case OE_NETCONF: return "NETCONF";
    case OE_PROTO: return "PROTO";
    case OE_REGEX: return "REGEX";
    case OE_UNIX: return "UNIX";
    case OE_SYSLOG: return "SYSLOG";
    case OE_XML: return "XML";
    case OE_JSON: return "JSON";
    case OE_RESTCONF: return "RESTCONF";
    case OE_PLUGIN: return "PLUGIN";
    case OE_YANG: return "YANG";
    case OE_FATAL: return "FATAL";
    case OE_SSL: return "SSL";
    default: return "UNKNOWN";
    }
}

int clixon_err_init(clixon_handle h)
{
    memset(&err_state, 0, sizeof(err_state));
    return 0;
}

int clixon_err_category(void)
{
    return err_state.category;
}

int clixon_err_subnr(void)
{
    return err_state.subnr;
}

char *clixon_err_reason(void)
{
    return err_state.reason;
}

char *clixon_err_str(void)
{
    return err_state.str;
}

int clixon_err_reset(void)
{
    memset(&err_state, 0, sizeof(err_state));
    return 0;
}

int clixon_err_fn(clixon_handle h, const char *fn, const int line, 
                  int category, int suberr, cxobj *xerr, 
                  const char *format, ...)
{
    va_list args;
    char msg[512];
    
    err_state.category = category;
    err_state.subnr = suberr;
    
    va_start(args, format);
    vsnprintf(msg, sizeof(msg), format, args);
    va_end(args);
    
    snprintf(err_state.reason, sizeof(err_state.reason), "%s", msg);
    snprintf(err_state.str, sizeof(err_state.str), 
             "[%s:%d] %s(%d): %s", 
             fn ? fn : "?", line, category_str(category), suberr, msg);
    
    /* Log to Zephyr */
    if (category == OE_FATAL) {
        LOG_ERR("%s", err_state.str);
    } else {
        LOG_WRN("%s", err_state.str);
    }
    
    return 0;
}

int netconf_err2cb(clixon_handle h, cxobj *xerr, cbuf *cberr)
{
    /* Simplified: just append current error */
    if (cberr && err_state.str[0]) {
        cbuf_append_str(cberr, err_state.str);
    }
    return 0;
}

void *clixon_err_save(void)
{
    /* Simplified: return pointer to current state */
    return &err_state;
}

int clixon_err_restore(void *handle)
{
    /* No-op in single-threaded embedded environment */
    return 0;
}

int clixon_err_cat_reg(enum clixon_err category, void *handle, clixon_cat_log_cb logfn)
{
    /* Not implemented for Zephyr */
    return 0;
}

int clixon_err_exit(void)
{
    memset(&err_state, 0, sizeof(err_state));
    return 0;
}
