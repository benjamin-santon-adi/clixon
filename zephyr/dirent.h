/*
 * Copyright (c) 2024 Benjamin Santon ADI
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Clixon dirent compatibility layer for Zephyr
 */

#ifndef CLIXON_ZEPHYR_DIRENT_H_
#define CLIXON_ZEPHYR_DIRENT_H_

#include <zephyr/kernel.h>
#include <zephyr/fs/fs.h>
#include <string.h>
#include <errno.h>

/* Directory entry structure compatible with POSIX dirent */
struct dirent {
	char d_name[256];
	unsigned char d_type;
};

/* Directory type values */
#define DT_UNKNOWN  0
#define DT_REG      8  /* Regular file */
#define DT_DIR      4  /* Directory */

/* Directory stream structure */
typedef struct {
	struct fs_dir_t zfs_dir;
	struct dirent current;
	int valid;
} DIR;

/* Directory operations */
DIR *opendir(const char *name);
struct dirent *readdir(DIR *dirp);
int closedir(DIR *dirp);
void rewinddir(DIR *dirp);

#endif /* CLIXON_ZEPHYR_DIRENT_H_ */
