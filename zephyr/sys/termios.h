/*
 * Copyright (c) 2024 Benjamin Santon ADI
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * termios.h stub for Zephyr - minimal terminal I/O support
 */

#ifndef CLIXON_ZEPHYR_SYS_TERMIOS_H_
#define CLIXON_ZEPHYR_SYS_TERMIOS_H_

#include <sys/types.h>

/* Terminal control structure - stub */
struct termios {
	unsigned int c_iflag;
	unsigned int c_oflag;
	unsigned int c_cflag;
	unsigned int c_lflag;
	unsigned char c_line;
	unsigned char c_cc[32];
};

/* Terminal control functions - stubs that do nothing */
static inline int tcgetattr(int fd, struct termios *termios_p) {
	(void)fd;
	(void)termios_p;
	return -1;
}

static inline int tcsetattr(int fd, int optional_actions, const struct termios *termios_p) {
	(void)fd;
	(void)optional_actions;
	(void)termios_p;
	return -1;
}

#endif /* CLIXON_ZEPHYR_SYS_TERMIOS_H_ */
