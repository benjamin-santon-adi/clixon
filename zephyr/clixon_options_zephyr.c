/*
 * Minimal Zephyr stub for clixon_options
 * Provides basic options handling without full file/XML/YANG parsing
 */

#ifdef HAVE_CONFIG_H
#include "clixon_config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <cligen/cligen.h>

/* clixon */
#include "clixon_queue.h"
#include "clixon_hash.h"
#include "clixon_handle.h"
#include "clixon_yang.h"
#include "clixon_xml.h"
#include "clixon_err.h"
#include "clixon_log.h"
#include "clixon_debug.h"
#include "clixon_options.h"
#include "clixon_map.h"
#include "clixon_string.h"

/* 
 * Basic option get/set implementation
 * In a full implementation, these would be populated from config files
 */

char *
clicon_option_str(clixon_handle h, const char *name)
{
    clicon_hash_t *copt;
    char          *val;
    
    if ((copt = clicon_options(h)) == NULL)
        return NULL;
    val = (char *)clicon_hash_value(copt, name, NULL);
    return val;
}

int
clicon_option_int(clixon_handle h, const char *name)
{
    char *val;
    
    if ((val = clicon_option_str(h, name)) == NULL)
        return -1;
    return atoi(val);
}

int
clicon_option_bool(clixon_handle h, const char *name)
{
    char *val;
    
    if ((val = clicon_option_str(h, name)) == NULL)
        return 0;
    return (strcmp(val, "true") == 0 || strcmp(val, "1") == 0);
}

int
clicon_option_set(clixon_handle h, const char *name, const char *value)
{
    clicon_hash_t *copt;
    clicon_hash_t result;
    
    if ((copt = clicon_options(h)) == NULL)
        return -1;
    result = clicon_hash_add(copt, name, (void *)value, strlen(value) + 1);
    return (result != NULL) ? 0 : -1;
}

int
clicon_option_del(clixon_handle h, const char *name)
{
    clicon_hash_t *copt;
    
    if ((copt = clicon_options(h)) == NULL)
        return -1;
    return clicon_hash_del(copt, name);
}

int
clicon_option_exists(clixon_handle h, const char *name)
{
    return (clicon_option_str(h, name) != NULL);
}

/* Minimal implementation - no file loading in embedded Zephyr */
int
clicon_options_main(clixon_handle h)
{
    /* Set some default options for Zephyr */
    clicon_option_set(h, "CLICON_CONFIGFILE", CLIXON_DEFAULT_CONFIG);
    clicon_option_set(h, "CLICON_YANG_DIR", YANG_INSTALLDIR);
    clicon_option_set(h, "CLICON_MODULE_LIBRARY_RFC7895", "true");
    
    return 0;
}

/* Stub - no config file dumping needed */
int
clicon_option_dump(clixon_handle h, int dblevel)
{
    if (dblevel) {
        clixon_log(h, 6, "Clixon options (embedded mode)"); /* 6 = LOG_INFO */
    }
    return 0;
}

/* Output callback stubs */
int 
clicon_output_set(clixon_handle h, clicon_output_cb *fn)
{
    /* Not implemented - use LOG functions instead */
    return 0;
}

clicon_output_cb *
clicon_output_get(clixon_handle h)
{
    /* Return NULL - will use default fprintf */
    return NULL;
}
