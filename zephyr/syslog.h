/*
 * Copyright (c) 2024 Benjamin Santon ADI
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Clixon POSIX compatibility layer for Zephyr
 * Provides syslog compatibility using Zephyr logging
 */

#ifndef CLIXON_ZEPHYR_SYSLOG_H_
#define CLIXON_ZEPHYR_SYSLOG_H_

#include <zephyr/logging/log.h>

/* syslog priority levels */
#define LOG_EMERG   0
#define LOG_ALERT   1
#define LOG_CRIT    2
#define LOG_ERR     3
#define LOG_WARNING 4
#define LOG_NOTICE  5
#define LOG_INFO    6
#define LOG_DEBUG   7

/* Map syslog to Zephyr logging */
#define syslog(priority, ...) \
	do { \
		switch (priority) { \
		case LOG_EMERG: \
		case LOG_ALERT: \
		case LOG_CRIT: \
		case LOG_ERR: \
			LOG_ERR(__VA_ARGS__); \
			break; \
		case LOG_WARNING: \
			LOG_WRN(__VA_ARGS__); \
			break; \
		case LOG_NOTICE: \
		case LOG_INFO: \
			LOG_INF(__VA_ARGS__); \
			break; \
		case LOG_DEBUG: \
			LOG_DBG(__VA_ARGS__); \
			break; \
		default: \
			LOG_INF(__VA_ARGS__); \
			break; \
		} \
	} while (0)

/* Stub functions for syslog open/close */
static inline void openlog(const char *ident, int option, int facility)
{
	/* No-op for Zephyr */
	ARG_UNUSED(ident);
	ARG_UNUSED(option);
	ARG_UNUSED(facility);
}

static inline void closelog(void)
{
	/* No-op for Zephyr */
}

#endif /* CLIXON_ZEPHYR_SYSLOG_H_ */
