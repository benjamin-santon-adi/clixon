# Clixon Zephyr Port Status

## Current Status

The Clixon library is being ported to Zephyr RTOS. This is work-in-progress with partial functionality.

### What's Working
- ✅ Zephyr module structure (module.yml, Kconfig, CMakeLists.txt)
- ✅ Clixon service with SYS_INIT auto-initialization
- ✅ POSIX compatibility layers for syslog (mapped to Zephyr logging)
- ✅ File system compatibility layer for dirent (using Zephyr FS API)
- ✅ Sample application demonstrating integration

### What's In Progress
- ⚠️ CLIgen library integration (stub header created, needs full implementation)
- ⚠️ Core clixon library compilation (many files excluded due to dependencies)

### External Dependencies

Clixon has the following external dependencies that need to be addressed:

1. **CLIgen** (Command Line Interface Generator)
   - Required by almost all clixon source files
   - Stub header created at `zephyr/cligen/cligen.h`
   - **Can be replaced**: See [CLIGEN_REPLACEMENT_ANALYSIS.md](CLIGEN_REPLACEMENT_ANALYSIS.md)
   - Options: 
     - Skip CLI features (use programmatic API only) ✅ Recommended for embedded
     - Port CLIgen library
     - Create adapter to Zephyr Shell subsystem
     - Fork Clixon for embedded use without CLI
   - Status: Analysis complete, architectural decision needed

2. **OpenSSL** 
   - Required by: `clixon_digest.c`
   - Needs: mbedTLS integration or OpenSSL port
   - Status: File currently excluded from build

3. **Dynamic Linking (dlfcn.h)**
   - Required by: `clixon_datastore.c`, `clixon_plugin.c`
   - Needs: Static plugin system for Zephyr
   - Status: Files currently excluded from build

### Build Configuration

#### Kconfig Options
- `CONFIG_CLIXON` - Enable Clixon library (enables POSIX_API, FILE_SYSTEM, LITTLEFS)
- `CONFIG_CLIXON_SERVICE` - Enable auto-init service via SYS_INIT
- `CONFIG_CLIXON_DEBUG` - Enable debug output
- `CONFIG_CLIXON_LOG_LEVEL_*` - Configure logging level

#### Requirements
- POSIX API support (`CONFIG_POSIX_API`)
- File system support (`CONFIG_FILE_SYSTEM_LITTLEFS`)
- Heap memory pool (recommended 16KB+)
- Main stack size (recommended 8KB+)

### Files Currently Excluded

The following source files are excluded from the build due to missing dependencies:

```
# Require CLIgen:
- clixon_autocli.c
- clixon_autocli_generate.c
- clixon_client.c
(and others requiring cbuf, cg_var, cvec types)

# Require dlfcn (dynamic linking):
- clixon_datastore.c
- clixon_plugin.c

# Require OpenSSL:
- clixon_digest.c
```

### Compatibility Layers

Located in `zephyr/` directory:

1. **syslog.h** - Maps syslog() to Zephyr LOG_*() macros
2. **dirent.h** / **dirent.c** - Implements POSIX dirent using Zephyr FS API
3. **cligen/cligen.h** - Stub header for CLIgen types (incomplete)

### Next Steps

1. **CLIgen Integration**
   - Option A: Port CLIgen library to Zephyr
   - Option B: Create minimal CLI compatibility layer
   - Option C: Disable CLI features, focus on XML/YANG core

2. **Crypto Support**
   - Integrate mbedTLS for digest/crypto functions
   - Or port OpenSSL subset needed by Clixon

3. **Plugin System**
   - Design static plugin system for Zephyr (no dlopen)
   - Use Zephyr iterable sections for plugin registration

4. **Parser Files**
   - Generate C files from flex/bison sources
   - Or port parser generators to run at build time

### Testing

To test the current state:

```bash
west build -b qemu_x86 modules/lib/clixon/zephyr/sample
```

Note: Build will fail with CLIgen-related errors. This is expected.

### Contributing

When adding compatibility layers:
- Do NOT modify upstream clixon code
- Place all Zephyr-specific code in `zephyr/` directory
- Use `#ifdef __ZEPHYR__` if upstream modifications are unavoidable
- Document all stubs and compatibility shims

## License

Clixon is dual-licensed (Apache 2.0 / GPL v2).
Zephyr integration code: Apache 2.0
