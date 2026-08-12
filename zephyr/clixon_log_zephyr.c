/*
 * Zephyr-specific logging for Clixon
 * Maps Clixon logging to Zephyr logging system
 */

#ifdef HAVE_CONFIG_H
#include "clixon_config.h"
#endif

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <stdio.h>
#include <stdarg.h>
#include <syslog.h>
#include <string.h>

LOG_MODULE_REGISTER(clixon_log, CONFIG_CLIXON_LOG_LEVEL);

#include <cligen/cligen.h>

/* Clixon headers */
#include "clixon_queue.h"
#include "clixon_hash.h"
#include "clixon_handle.h"
#include "clixon_xml.h"
#include "clixon_log.h"

static uint16_t log_flags = CLIXON_LOG_SYSLOG;
static size_t log_string_limit = 1024;

static const struct {
    int key;
    const char *str;
} logdstmap[] = {
    {CLIXON_LOG_SYSLOG, "syslog"},
    {CLIXON_LOG_STDERR, "stderr"},
    {CLIXON_LOG_STDOUT, "stdout"},
    {CLIXON_LOG_FILE,   "file"},
    {0, NULL}
};

const char *clixon_logdst_key2str(int keyword)
{
    for (int i = 0; logdstmap[i].str; i++) {
        if (logdstmap[i].key == keyword)
            return logdstmap[i].str;
    }
    return NULL;
}

int clixon_logdst_str2key(const char *str)
{
    for (int i = 0; logdstmap[i].str; i++) {
        if (strcmp(logdstmap[i].str, str) == 0)
            return logdstmap[i].key;
    }
    return -1;
}

int clixon_log_init(clixon_handle h, const char *ident, int upto, uint16_t flags)
{
    log_flags = flags;
    LOG_INF("Clixon logging initialized: %s", ident ? ident : "clixon");
    return 0;
}

int clixon_log_exit(void)
{
    return 0;
}

int clixon_log_opt(char c)
{
    /* Handle logging command-line options if needed */
    return 0;
}

int clixon_log_file(const char *filename)
{
    /* File logging not supported in Zephyr */
    return -1;
}

int clixon_log_string_limit_set(size_t sz)
{
    log_string_limit = sz;
    return 0;
}

size_t clixon_log_string_limit_get(void)
{
    return log_string_limit;
}

uint16_t clixon_logflags_get(void)
{
    return log_flags;
}

int clixon_logflags_set(uint16_t flags)
{
    log_flags = flags;
    return 0;
}

int clixon_log_str(int level, char *msg)
{
    if (!msg) return 0;
    
    /* Map syslog levels to Zephyr logging 
     * Use Z_LOG_* to avoid conflict with syslog.h defines */
    switch (level) {
    case 0: /* LOG_EMERG */
    case 1: /* LOG_ALERT */
    case 2: /* LOG_CRIT */
    case 3: /* LOG_ERR */
        Z_LOG(LOG_LEVEL_ERR, "%s", msg);
        break;
    case 4: /* LOG_WARNING */
        Z_LOG(LOG_LEVEL_WRN, "%s", msg);
        break;
    case 5: /* LOG_NOTICE */
    case 6: /* LOG_INFO */
        Z_LOG(LOG_LEVEL_INF, "%s", msg);
        break;
    case 7: /* LOG_DEBUG */
        Z_LOG(LOG_LEVEL_DBG, "%s", msg);
        break;
    default:
        Z_LOG(LOG_LEVEL_INF, "%s", msg);
    }
    return 0;
}

int clixon_log_fn(clixon_handle h, int user, int level, cxobj *x, 
                  const char *format, ...)
{
    va_list args;
    char msg[512];
    
    va_start(args, format);
    vsnprintf(msg, sizeof(msg), format, args);
    va_end(args);
    
    return clixon_log_str(level, msg);
}

int clixon_log_timestamp(clixon_handle h, uint64_t *timestamp, 
                        const char *format, ...)
{
    va_list args;
    char msg[512];
    
    if (timestamp) {
        *timestamp = k_uptime_get();
    }
    
    va_start(args, format);
    vsnprintf(msg, sizeof(msg), format, args);
    va_end(args);
    
    LOG_INF("%s", msg);
    return 0;
}
