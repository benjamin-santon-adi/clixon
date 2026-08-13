# NETCONF Clixon Integration Guide

## Current Implementation Status

The current NETCONF server implementation in `clixon_service.c` is a **simplified demonstration** that uses hardcoded XML responses. This is suitable for:
- Initial testing and development
- Understanding NETCONF protocol basics  
- GUI client development
- Simple read-only operations

## Proper Clixon Integration (TODO)

For production use, the server should leverage Clixon's native NETCONF handling:

### Required Components

1. **XML Parsing**
   ```c
   #include "clixon_xml.h"
   
   // Parse incoming NETCONF message
   cxobj *xn = NULL;
   if (clixon_xml_parse_string(recv_buf, YB_RPC, NULL, &xn, NULL) < 0) {
       // Handle parse error
   }
   ```

2. **RPC Dispatch**
   ```c
   #include "netconf_rpc.h"
   
   // Process RPC using Clixon's native handler
   cxobj *xret = NULL;
   int eof = 0;
   if (netconf_rpc_dispatch(clixon_service.handle, xn, &xret, &eof) < 0) {
       // Handle dispatch error
   }
   ```

3. **Response Generation**
   ```c
   // Serialize XML response to string
   cbuf *cb = cbuf_new();
   clixon_xml2cbuf(cb, xret, 0, 0, NULL, -1, 0);
   char *reply = cbuf_get(cb);
   
   // Send over TLS
   mbedtls_ssl_write(&ssl, reply, strlen(reply));
   
   // Cleanup
   cbuf_free(cb);
   xml_free(xret);
   ```

4. **Hello Message**
   ```c
   #include "clixon_netconf_lib.h"
   
   // Generate proper hello with capabilities
   cbuf *cb = cbuf_new();
   netconf_hello_server(clixon_service.handle, cb, session_id);
   char *hello = cbuf_get(cb);
   ```

### YANG Module Loading

For proper operation, YANG modules must be loaded:

```c
// In clixon_service_init()

// Set YANG directory
clicon_option_str_set(h, "CLICON_YANG_DIR", "/path/to/yang");

// Load YANG modules
if (yang_spec_load_dir(h, clicon_yang_dir(h)) < 0) {
    LOG_ERR("Failed to load YANG modules");
    return -1;
}

// Load standard modules
if (netconf_module_load(h) < 0) {
    LOG_ERR("Failed to load NETCONF monitoring module");
    return -1;
}
```

### Datastore Configuration

Clixon requires a datastore backend. Options for Zephyr:

#### Option 1: Memory-only (volatile)
```c
clicon_option_str_set(h, "CLICON_XMLDB_DIR", ":memory:");
clicon_option_str_set(h, "CLICON_XMLDB_FORMAT", "xml");
```

#### Option 2: NVS (Non-Volatile Storage)
```c
// Use Zephyr NVS for persistence
clicon_option_str_set(h, "CLICON_XMLDB_DIR", "/nvs/clixon");
clicon_option_str_set(h, "CLICON_XMLDB_PLUGIN", "nvs");
```

#### Option 3: LittleFS
```c
// Use LittleFS mount point
clicon_option_str_set(h, "CLICON_XMLDB_DIR", "/lfs/clixon");
clicon_option_str_set(h, "CLICON_XMLDB_FORMAT", "xml");
```

### Required Configuration Options

```c
// Application name
clicon_option_str_set(h, "CLICON_CONFIGFILE", "/etc/clixon/netconf.xml");
clicon_option_str_set(h, "CLICON_APPNAME", "netconf");

// Feature settings
clicon_option_bool_set(h, "CLICON_MODULE_SET_ID", 1);
clicon_option_bool_set(h, "CLICON_FEATURE", 1);

// Namespace handling
clicon_option_str_set(h, "CLICON_NAMESPACE_MAP", "urn:ietf:params:xml:ns:yang:ietf-netconf-monitoring netconf-monitoring");

// RPC callbacks
clicon_option_str_set(h, "CLICON_BACKEND_DIR", "/usr/local/lib/clixon/backend");
```

### Memory Considerations

Full Clixon integration requires significant memory:
- **YANG parser**: ~50-100KB RAM
- **XML tree storage**: ~10-50KB per datastore
- **Transaction management**: ~20-30KB
- **Plugin system**: ~20-40KB

**Minimum recommended**:
- Flash: 1.5-2MB for code + YANG modules
- RAM: 256-512KB depending on data size

### Zephyr-Specific Adaptations

Some Clixon functions need Zephyr adaptation:

1. **File I/O**: Replace POSIX file operations with Zephyr filesystem APIs
2. **Process management**: Clixon plugins use fork/exec - not applicable on Zephyr
3. **Signal handling**: Use Zephyr event system instead
4. **Syslog**: Map to Zephyr logging system

### Incremental Integration Steps

1. **Phase 1**: Basic XML parsing
   - Parse incoming RPCs to cxobj
   - Serialize responses from cxobj
   - Keep hardcoded logic for now

2. **Phase 2**: Datastore
   - Initialize Clixon datastore
   - Implement NVS or filesystem backend
   - Load/store configuration

3. **Phase 3**: YANG modules
   - Load standard YANG modules
   - Enable YANG validation
   - Schema-aware operations

4. **Phase 4**: RPC dispatch
   - Use netconf_rpc_dispatch()
   - Implement standard operations (get-config, edit-config, etc.)
   - Transaction support

5. **Phase 5**: Advanced features
   - Notifications (RFC 5277)
   - Partial lock (RFC 5717)
   - YANG library (RFC 7895)

## Benefits of Full Integration

1. **Standards Compliance**: Proper NETCONF 1.0/1.1 implementation
2. **YANG Validation**: Schema-aware configuration management
3. **Transaction Support**: ACID properties for configuration changes
4. **Extensibility**: Easy to add new YANG modules
5. **Rollback**: Candidate datastore with commit/rollback
6. **Locking**: Proper datastore locking mechanisms

## Current Limitations

The simplified implementation:
- ✗ No YANG validation
- ✗ No persistent storage (data lost on reboot)
- ✗ No transaction support
- ✗ Limited RPC operations (only get-config, get, close-session)
- ✗ No edit-config implementation
- ✗ No candidate datastore
- ✗ No locking mechanisms

## References

- [Clixon Documentation](https://clixon-docs.readthedocs.io/)
- [RFC 6241 - NETCONF Protocol](https://www.rfc-editor.org/rfc/rfc6241.html)
- [RFC 6022 - NETCONF Monitoring](https://www.rfc-editor.org/rfc/rfc6022.html)
- [RFC 7950 - YANG 1.1](https://www.rfc-editor.org/rfc/rfc7950.html)

## Contributing

To contribute to full Clixon integration:
1. Review Clixon apps/netconf implementation
2. Create Zephyr-compatible datastore backend
3. Port XML parsing to use less memory
4. Test with standard NETCONF test suites
