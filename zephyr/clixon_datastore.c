/*
 * Copyright (c) 2024 Benjamin Santon ADI
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Clixon Datastore Backend for Zephyr
 * Provides LittleFS integration for persistent configuration storage
 */

#include <zephyr/kernel.h>
#include <zephyr/fs/fs.h>
#include <zephyr/fs/littlefs.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include <string.h>
#include <errno.h>

#ifdef CONFIG_CLIXON
#include "clixon_handle.h"
#include "clixon_options.h"
#endif

LOG_MODULE_REGISTER(clixon_datastore, CONFIG_CLIXON_LOG_LEVEL);

#ifdef CONFIG_CLIXON_DATASTORE

/* Datastore state */
static struct {
	bool initialized;
	const char *mount_point;
	const char *xmldb_dir;
} datastore_state = {
	.initialized = false,
	.mount_point = "/lfs",
	.xmldb_dir = CONFIG_CLIXON_XMLDB_DIR,
};

/**
 * @brief Create directory recursively
 * 
 * @param path Directory path to create
 * @return 0 on success, negative errno on failure
 */
static int mkdir_recursive(const char *path)
{
	char tmp[256];
	char *p = NULL;
	size_t len;
	int ret;

	snprintf(tmp, sizeof(tmp), "%s", path);
	len = strlen(tmp);
	
	/* Remove trailing slash */
	if(tmp[len - 1] == '/') {
		tmp[len - 1] = 0;
	}

	/* Create directories recursively */
	for(p = tmp + 1; *p; p++) {
		if(*p == '/') {
			*p = 0;
			ret = fs_mkdir(tmp);
			if (ret < 0 && ret != -EEXIST) {
				LOG_ERR("Failed to create directory %s: %d", tmp, ret);
				return ret;
			}
			*p = '/';
		}
	}
	
	/* Create final directory */
	ret = fs_mkdir(tmp);
	if (ret < 0 && ret != -EEXIST) {
		LOG_ERR("Failed to create directory %s: %d", tmp, ret);
		return ret;
	}

	return 0;
}

/**
 * @brief Initialize Clixon datastore directories
 * 
 * Creates the required directory structure:
 * - /lfs/clixon/running      - Running configuration
 * - /lfs/clixon/candidate    - Candidate configuration  
 * - /lfs/clixon/startup      - Startup configuration
 * - /lfs/clixon/tmp          - Temporary files
 * 
 * @return 0 on success, negative errno on failure
 */
static int init_datastore_dirs(void)
{
	int ret;
	char path[256];

	LOG_INF("Creating Clixon datastore directories...");

	/* Create base directory */
	ret = mkdir_recursive(datastore_state.xmldb_dir);
	if (ret < 0) {
		LOG_ERR("Failed to create base directory %s: %d", 
		        datastore_state.xmldb_dir, ret);
		return ret;
	}

	/* Create datastore subdirectories */
	const char *subdirs[] = {"running", "candidate", "startup", "tmp"};
	for (int i = 0; i < ARRAY_SIZE(subdirs); i++) {
		snprintf(path, sizeof(path), "%s/%s", 
		         datastore_state.xmldb_dir, subdirs[i]);
		ret = mkdir_recursive(path);
		if (ret < 0) {
			LOG_ERR("Failed to create directory %s: %d", path, ret);
			return ret;
		}
		LOG_DBG("Created directory: %s", path);
	}

	LOG_INF("Datastore directories created successfully");
	return 0;
}

/**
 * @brief Initialize empty datastore files if they don't exist
 * 
 * @return 0 on success, negative errno on failure
 */
static int init_datastore_files(void)
{
	int ret;
	char filepath[256];
	struct fs_file_t file;
	const char *datastores[] = {"running", "candidate", "startup"};

	LOG_INF("Initializing datastore files...");

	fs_file_t_init(&file);

	for (int i = 0; i < ARRAY_SIZE(datastores); i++) {
		snprintf(filepath, sizeof(filepath), "%s/%s/db.xml",
		         datastore_state.xmldb_dir, datastores[i]);

		/* Check if file exists */
		ret = fs_open(&file, filepath, FS_O_READ);
		if (ret == 0) {
			/* File exists, close it */
			fs_close(&file);
			LOG_DBG("Datastore %s already exists", datastores[i]);
			continue;
		}

		/* Create new empty datastore */
		ret = fs_open(&file, filepath, FS_O_CREATE | FS_O_WRITE);
		if (ret < 0) {
			LOG_ERR("Failed to create datastore %s: %d", datastores[i], ret);
			return ret;
		}

		/* Write empty XML document */
		const char *empty_xml = 
			"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
			"<config xmlns=\"http://clicon.org/config\"/>\n";
		
		ret = fs_write(&file, empty_xml, strlen(empty_xml));
		if (ret < 0) {
			LOG_ERR("Failed to write to datastore %s: %d", datastores[i], ret);
			fs_close(&file);
			return ret;
		}

		fs_close(&file);
		LOG_INF("Created empty datastore: %s", datastores[i]);
	}

	return 0;
}

/**
 * @brief Initialize Clixon datastore backend
 * 
 * @param h Clixon handle (can be NULL for standalone init)
 * @return 0 on success, negative errno on failure
 */
int clixon_datastore_init(clixon_handle h)
{
	int ret;

	if (datastore_state.initialized) {
		LOG_WRN("Datastore already initialized");
		return 0;
	}

	LOG_INF("Initializing Clixon datastore backend");
	LOG_INF("Mount point: %s", datastore_state.mount_point);
	LOG_INF("XMLDB directory: %s", datastore_state.xmldb_dir);

	/* Wait for filesystem to be mounted */
	k_sleep(K_MSEC(100));

	/* Verify mount point is accessible */
	struct fs_dir_t dir;
	fs_dir_t_init(&dir);
	ret = fs_opendir(&dir, datastore_state.mount_point);
	if (ret < 0) {
		LOG_ERR("Mount point %s not accessible: %d", 
		        datastore_state.mount_point, ret);
		return ret;
	}
	fs_closedir(&dir);
	LOG_DBG("Mount point verified");

	/* Create directory structure */
	ret = init_datastore_dirs();
	if (ret < 0) {
		LOG_ERR("Failed to initialize datastore directories");
		return ret;
	}

	/* Initialize datastore files */
	ret = init_datastore_files();
	if (ret < 0) {
		LOG_ERR("Failed to initialize datastore files");
		return ret;
	}

#ifdef CONFIG_CLIXON
	/* Configure Clixon handle if provided */
	if (h != NULL) {
		LOG_INF("Configuring Clixon options for datastore");
		
		/* Set datastore directory */
		if (clicon_option_str_set(h, "CLICON_XMLDB_DIR", 
		                          datastore_state.xmldb_dir) < 0) {
			LOG_ERR("Failed to set CLICON_XMLDB_DIR");
			return -EINVAL;
		}

		/* Set datastore format */
		if (clicon_option_str_set(h, "CLICON_XMLDB_FORMAT", 
		                          CONFIG_CLIXON_XMLDB_FORMAT) < 0) {
			LOG_ERR("Failed to set CLICON_XMLDB_FORMAT");
			return -EINVAL;
		}

		/* Enable datastore cache */
		if (clicon_option_bool_set(h, "CLICON_XMLDB_CACHE", 1) < 0) {
			LOG_ERR("Failed to enable XMLDB cache");
			return -EINVAL;
		}

		LOG_INF("Clixon datastore options configured");
	}
#endif

	datastore_state.initialized = true;
	LOG_INF("Clixon datastore initialized successfully");

	return 0;
}

/**
 * @brief Check if datastore is initialized
 * 
 * @return true if initialized, false otherwise
 */
bool clixon_datastore_is_initialized(void)
{
	return datastore_state.initialized;
}

/**
 * @brief Get datastore directory path
 * 
 * @return Datastore directory path
 */
const char *clixon_datastore_get_dir(void)
{
	return datastore_state.xmldb_dir;
}

#endif /* CONFIG_CLIXON_DATASTORE */
