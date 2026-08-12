/*
 * Copyright (c) 2024 Benjamin Santon ADI
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Clixon dirent compatibility implementation for Zephyr
 */

#include "dirent.h"
#include <stdlib.h>

DIR *opendir(const char *name)
{
	DIR *dirp = (DIR *)malloc(sizeof(DIR));
	if (!dirp) {
		errno = ENOMEM;
		return NULL;
	}

	memset(dirp, 0, sizeof(DIR));
	
	int ret = fs_opendir(&dirp->zfs_dir, name);
	if (ret < 0) {
		free(dirp);
		errno = -ret;
		return NULL;
	}

	dirp->valid = 0;
	return dirp;
}

struct dirent *readdir(DIR *dirp)
{
	if (!dirp) {
		errno = EBADF;
		return NULL;
	}

	struct fs_dirent entry;
	int ret = fs_readdir(&dirp->zfs_dir, &entry);
	
	if (ret < 0) {
		errno = -ret;
		return NULL;
	}

	if (entry.name[0] == '\0') {
		/* End of directory */
		return NULL;
	}

	/* Copy to dirent structure */
	strncpy(dirp->current.d_name, entry.name, sizeof(dirp->current.d_name) - 1);
	dirp->current.d_name[sizeof(dirp->current.d_name) - 1] = '\0';
	
	/* Map Zephyr file type to POSIX d_type */
	if (entry.type == FS_DIR_ENTRY_DIR) {
		dirp->current.d_type = DT_DIR;
	} else if (entry.type == FS_DIR_ENTRY_FILE) {
		dirp->current.d_type = DT_REG;
	} else {
		dirp->current.d_type = DT_UNKNOWN;
	}

	dirp->valid = 1;
	return &dirp->current;
}

int closedir(DIR *dirp)
{
	if (!dirp) {
		errno = EBADF;
		return -1;
	}

	int ret = fs_closedir(&dirp->zfs_dir);
	free(dirp);
	
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return 0;
}

void rewinddir(DIR *dirp)
{
	if (!dirp) {
		return;
	}

	/* Zephyr FS doesn't have rewinddir, would need to close and reopen */
	dirp->valid = 0;
}
