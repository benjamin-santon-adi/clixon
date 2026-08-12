/*
 * Zephyr-specific Clixon configuration
 * This file provides configuration macros that would normally be generated
 * by autoconf in a Unix build
 */

#ifndef _CLIXON_CONFIG_H_
#define _CLIXON_CONFIG_H_

/* Version information */
#define CLIXON_VERSION "6.0.0-zephyr"
#define CLIXON_VERSION_MAJOR 6
#define CLIXON_VERSION_MINOR 0
#define CLIXON_VERSION_PATCH 0

/* Default paths - adapted for Zephyr */
#define CLIXON_DEFAULT_CONFIG "/clixon.xml"
#define YANG_INSTALLDIR "/yang"
#define YANG_STANDARD_DIR "/yang/standard"

/* Feature flags */
#define HAVE_STRNDUP 1
#define HAVE_STRVERSCMP 0  /* Not available in Zephyr/picolibc */
#define HAVE_STRPTIME 1

/* Sizes and limits */
#define MAXPATHLEN 256
#ifndef PATH_MAX
#define PATH_MAX 256
#endif

/* Plugin support - disabled for Zephyr (no dlopen) */
#undef HAVE_LIBDL

/* OpenSSL/crypto - will use mbedTLS instead */
#undef HAVE_LIBSSL

/* System features */
#define HAVE_SYSLOG_H 1  /* We provide a compatibility layer */
#define HAVE_FCNTL_H 1
#define HAVE_UNISTD_H 1

/* Disabled features for embedded */
#undef HAVE_GRPC
#undef HAVE_LIBNGHTTP2

/* Clixon feature flags */
#define CLIXON_DATASTORE_CACHE 1
#define CLIXON_AUTOCLI 0  /* Disable auto-CLI for now */

#endif /* _CLIXON_CONFIG_H_ */
