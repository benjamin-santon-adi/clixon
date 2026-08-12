/*
 * Zephyr-specific debug support for Clixon
 */

#ifdef HAVE_CONFIG_H
#include "clixon_config.h"
#endif

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

LOG_MODULE_REGISTER(clixon_debug, CONFIG_CLIXON_LOG_LEVEL);

#include <cligen/cligen.h>

/* Clixon headers */
#include "clixon_queue.h"
#include "clixon_hash.h"
#include "clixon_handle.h"
#include "clixon_xml.h"
#include "clixon_debug.h"

static int debug_flags = 0;
static size_t explicit_trunc = CLIXON_DBG_EXPLICIT_TRUNC_DEFAULT;

int clixon_debug_init(clixon_handle h, int flags)
{
    debug_flags = flags;
    return 0;
}

int clixon_debug_exit(void)
{
    return 0;
}

int clixon_debug_get(void)
{
    return debug_flags;
}

int clixon_debug_set(int flags)
{
    debug_flags = flags;
    return 0;
}

int clixon_debug_fn(clixon_handle h, const char *fn, const int line,
                    int dbglevel, cxobj *x, const char *format, ...)
{
    /* Only log if debug flag matches */
    if (!(debug_flags & dbglevel))
        return 0;
        
    va_list args;
    char msg[512];
    int detail = (dbglevel & CLIXON_DBG_DMASK) >> CLIXON_DBG_DSHIFT;
    
    va_start(args, format);
    vsnprintf(msg, sizeof(msg), format, args);
    va_end(args);
    
    LOG_DBG("[%s:%d] %s", fn ? fn : "?", line, msg);
    return 0;
}

size_t clixon_debug_explicit_trunc_get(void)
{
    return explicit_trunc;
}

int clixon_debug_explicit_trunc_set(size_t len)
{
    explicit_trunc = len;
    return 0;
}

const char *clixon_debug_key2str(int keyword)
{
    switch(keyword) {
    case CLIXON_DBG_DEFAULT: return "default";
    case CLIXON_DBG_MSG: return "msg";
    case CLIXON_DBG_INIT: return "init";
    case CLIXON_DBG_XML: return "xml";
    case CLIXON_DBG_XPATH: return "xpath";
    case CLIXON_DBG_YANG: return "yang";
    case CLIXON_DBG_BACKEND: return "backend";
    case CLIXON_DBG_CLI: return "cli";
    case CLIXON_DBG_NETCONF: return "netconf";
    case CLIXON_DBG_RESTCONF: return "restconf";
    default: return "unknown";
    }
}

int clixon_debug_str2key(const char *str)
{
    if (strcmp(str, "default") == 0) return CLIXON_DBG_DEFAULT;
    if (strcmp(str, "msg") == 0) return CLIXON_DBG_MSG;
    if (strcmp(str, "init") == 0) return CLIXON_DBG_INIT;
    if (strcmp(str, "xml") == 0) return CLIXON_DBG_XML;
    if (strcmp(str, "xpath") == 0) return CLIXON_DBG_XPATH;
    if (strcmp(str, "yang") == 0) return CLIXON_DBG_YANG;
    if (strcmp(str, "backend") == 0) return CLIXON_DBG_BACKEND;
    if (strcmp(str, "cli") == 0) return CLIXON_DBG_CLI;
    if (strcmp(str, "netconf") == 0) return CLIXON_DBG_NETCONF;
    if (strcmp(str, "restconf") == 0) return CLIXON_DBG_RESTCONF;
    return -1;
}
