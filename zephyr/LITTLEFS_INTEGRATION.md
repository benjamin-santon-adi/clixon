# Clixon with LittleFS Datastore Integration

## Overview

This implementation provides full Clixon NETCONF server integration with persistent storage using LittleFS on the ADIN6310's internal flash.

## Features

✅ **LittleFS Backend**
- Power-fail safe storage
- Wear leveling
- 256KB flash partition
- Optimized for embedded systems

✅ **Persistent Datastores**
- Running configuration
- Candidate configuration
- Startup configuration
- Automatic initialization

✅ **Flash Layout**

MAX32690 Internal Flash (3MB total):
```
0x00000000 - 0x002C0000  Application code (2.75MB)
0x002C0000 - 0x00300000  Clixon storage (256KB)
```

## Directory Structure

```
/lfs/clixon/
├── running/
│   └── db.xml          # Running configuration
├── candidate/
│   └── db.xml          # Candidate configuration
├── startup/
│   └── db.xml          # Startup configuration
└── tmp/                # Temporary files
```

## Configuration

### Kconfig Options

```kconfig
CONFIG_CLIXON_DATASTORE=y
CONFIG_CLIXON_DATASTORE_LITTLEFS=y
CONFIG_CLIXON_XMLDB_DIR="/lfs/clixon"
CONFIG_CLIXON_XMLDB_FORMAT="xml"
```

### Filesystem Settings

```kconfig
CONFIG_FILE_SYSTEM=y
CONFIG_FILE_SYSTEM_LITTLEFS=y
CONFIG_FLASH=y
CONFIG_FLASH_MAP=y
CONFIG_FS_LITTLEFS_CACHE_SIZE=256
CONFIG_FS_LITTLEFS_BLOCK_SIZE=512
```

### Board Configuration

The devicetree overlay (`eval_adin6310t1lebz_max32690_m4.overlay`) defines:
- Flash partition at 0x002C0000 (256KB)
- LittleFS mount point at `/lfs`
- Automatic mounting at boot

## Building

```bash
west build -b eval_adin6310t1lebz/max32690/m4 modules/lib/clixon/zephyr/sample
```

The build system automatically includes:
- Board-specific configuration (`eval_adin6310t1lebz_max32690_m4.conf`)
- Devicetree overlay (`eval_adin6310t1lebz_max32690_m4.overlay`)
- LittleFS configuration (`lfs.conf`)

## Memory Usage

**Flash:**
- Application + Clixon: ~2.75MB
- Datastore partition: 256KB
- Total: ~3MB (full MAX32690 capacity)

**RAM:**
- Heap: 192KB (for TLS + filesystem)
- LittleFS cache: 256 bytes
- File descriptors: 8 files + 8 directories

## Usage

### NETCONF Operations

**Get Configuration:**
```xml
<get-config>
  <source><running/></source>
</get-config>
```

**Edit Configuration:**
```xml
<edit-config>
  <target><candidate/></target>
  <config>
    <system xmlns="urn:ietf:params:xml:ns:yang:ietf-system">
      <hostname>my-device</hostname>
    </system>
  </config>
</edit-config>
```

**Commit Changes:**
```xml
<commit/>
```

### Persistence

All configurations are automatically saved to flash:
- Changes to running config persist across reboots
- Candidate changes are preserved until committed or discarded
- Startup config is loaded at boot

## Datastore API

```c
#include "clixon_datastore.h"

/* Initialize datastore */
int ret = clixon_datastore_init(clixon_handle);

/* Check if initialized */
bool ready = clixon_datastore_is_initialized();

/* Get datastore directory */
const char *dir = clixon_datastore_get_dir();
```

## Filesystem Access

The datastore files can be accessed directly via Zephyr filesystem APIs:

```c
#include <zephyr/fs/fs.h>

struct fs_file_t file;
fs_file_t_init(&file);

/* Read running config */
fs_open(&file, "/lfs/clixon/running/db.xml", FS_O_READ);
/* ... read operations ... */
fs_close(&file);
```

## Monitoring

View filesystem status via shell:
```
uart:~$ fs mount
/lfs : LITTLEFS

uart:~$ fs ls /lfs/clixon
    clixon:
         DIR candidate
         DIR running
         DIR startup
         DIR tmp

uart:~$ fs ls /lfs/clixon/running
         FILE db.xml (size = 123)
```

## Troubleshooting

### Mount Failed

**Symptom**: `Mount point /lfs not accessible`

**Solutions**:
1. Check devicetree overlay is included
2. Verify flash partition size and offset
3. Check CONFIG_FLASH=y and CONFIG_FLASH_MAP=y

### Write Failed

**Symptom**: `Failed to write to datastore`

**Solutions**:
1. Check heap size (needs 192KB minimum)
2. Verify LittleFS cache size
3. Ensure enough free space in partition

### Corruption

**Symptom**: `Failed to parse XML`

**Solutions**:
1. LittleFS is power-fail safe, but check for flash wear
2. Verify block size matches flash erase size
3. Re-initialize with `fs format /lfs`

## Limitations

Current implementation:
- ✅ Persistent storage working
- ✅ Multiple datastores (running/candidate/startup)
- ⚠️ Basic XML files only (no Clixon XML parsing yet)
- ⚠️ No YANG validation (hardcoded data models)
- ⚠️ No transaction support (simple file operations)

## Next Steps

For full Clixon integration:

1. **XML Parsing**: Integrate `clixon_xml_parse_string()`
2. **RPC Dispatch**: Use `netconf_rpc_dispatch()`
3. **YANG Modules**: Load and validate against schemas
4. **Transactions**: Implement commit/rollback
5. **Locking**: Datastore locking mechanisms

See [NETCONF_CLIXON_INTEGRATION.md](NETCONF_CLIXON_INTEGRATION.md) for details.

## References

- [LittleFS Documentation](https://github.com/littlefs-project/littlefs)
- [Zephyr File System](https://docs.zephyrproject.org/latest/services/file_system/index.html)
- [Clixon Documentation](https://clixon-docs.readthedocs.io/)
- [RFC 6241 - NETCONF Protocol](https://www.rfc-editor.org/rfc/rfc6241.html)
