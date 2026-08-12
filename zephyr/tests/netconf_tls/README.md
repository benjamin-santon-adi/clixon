# NETCONF over TLS Twister Test

This test validates the Clixon NETCONF server with TLS encryption using Zephyr's Twister test framework and pytest.

## Test Overview

The test suite validates:
- ✅ NETCONF server starts and listens on port 830
- ✅ TLS handshake completes successfully
- ✅ Server sends valid NETCONF hello message
- ✅ Complete hello exchange (client ↔ server)
- ✅ Secure cipher suite usage (TLS 1.2+)
- ✅ Multiple sequential connections
- ✅ Connection timeout behavior

## Running the Tests

### Run with Twister

From the Zephyr workspace root:

```bash
# Run all NETCONF TLS tests
./zephyr/scripts/twister -T modules/lib/clixon/zephyr/tests/netconf_tls

# Run on specific platform
./zephyr/scripts/twister -p native_posix -T modules/lib/clixon/zephyr/tests/netconf_tls

# Run with verbose output
./zephyr/scripts/twister -v -T modules/lib/clixon/zephyr/tests/netconf_tls

# Run and keep artifacts
./zephyr/scripts/twister --inline-logs -T modules/lib/clixon/zephyr/tests/netconf_tls
```

### Run Manually (for debugging)

```bash
# Build the test
west build -b native_posix modules/lib/clixon/zephyr/tests/netconf_tls

# Run the application (in one terminal)
west build -t run

# Run pytest manually (in another terminal)
cd modules/lib/clixon/zephyr/tests/netconf_tls
pytest pytest/test_netconf_tls.py -v -s
```

## Test Structure

```
tests/netconf_tls/
├── CMakeLists.txt              # Build configuration
├── prj.conf                    # Kconfig settings
├── testcase.yaml              # Twister test definition
├── README.md                  # This file
├── src/
│   └── main.c                 # Test application
└── pytest/
    ├── conftest.py           # Pytest configuration
    └── test_netconf_tls.py   # Test cases
```

## Test Cases

### 1. test_server_listening
Verifies that the NETCONF server is listening on port 830.

**Expected:** TCP connection succeeds

### 2. test_tls_handshake
Tests TLS handshake with the server using self-signed certificates.

**Expected:** TLS handshake completes without errors

### 3. test_receive_hello
Validates that the server sends a properly formatted NETCONF hello message.

**Expected:** Hello contains:
- XML declaration
- NETCONF base:1.0 namespace
- Capabilities list
- Base 1.0 capability
- Message delimiter `]]>]]>`

### 4. test_hello_exchange
Tests complete hello exchange: receive server hello, send client hello.

**Expected:** Both directions succeed

### 5. test_tls_cipher_suite
Verifies that secure TLS version and cipher suite are used.

**Expected:**
- TLS 1.2 or higher
- No weak ciphers (DES, RC4, MD5, NULL, anon)

### 6. test_multiple_connections
Tests that the server can handle multiple sequential connections.

**Expected:** 3 consecutive connections all succeed

### 7. test_connection_timeout
Validates connection behavior with idle connections.

**Expected:** Connection remains stable or closes gracefully

## Network Configuration

The test uses the following network settings:

- **Server IP:** 192.0.2.1 (Zephyr device)
- **Client IP:** 192.0.2.2 (Test host)
- **Port:** 830 (NETCONF standard)
- **Subnet:** 192.0.2.0/24 (TEST-NET-1, RFC 5737)

These are automatically configured via `CONFIG_NET_CONFIG_SETTINGS=y`.

## TLS Configuration

The test uses the embedded test certificates from `test_certs/`:

- **CA Certificate:** `ca-cert.pem`
- **Server Certificate:** Embedded in `server_cert.h`
- **Server Key:** Embedded in `server_key.h`

⚠️ **Note:** These are self-signed test certificates. The Python client either:
1. Loads `ca-cert.pem` for verification, or
2. Disables certificate verification if CA cert is not found

## Dependencies

Python packages required for pytest:
```bash
# None - uses only Python standard library (socket, ssl)
# No ncclient needed - we use a simple custom client
```

## Troubleshooting

### Test hangs or times out

**Problem:** Pytest hangs waiting for connection

**Solutions:**
1. Check that the Zephyr app is running: `west build -t run`
2. Verify network configuration in logs
3. Try increasing timeout in `test_netconf_tls.py`
4. Run with native_posix for better networking

### TLS handshake fails

**Problem:** `SSLError: [SSL] record layer failure`

**Solutions:**
1. Verify certificates were generated: `ls test_certs/`
2. Regenerate certificates: `python der_to_c.py`
3. Rebuild application to include new certs
4. Check that `CONFIG_CLIXON_NETCONF_TLS=y`

### Connection refused

**Problem:** Cannot connect to 192.0.2.1:830

**Solutions:**
1. Verify server started: check logs for "Test ready"
2. Check IP address configuration in logs
3. Verify platform supports networking (native_posix recommended)
4. Try with qemu_x86 and proper network setup

### Certificate verification fails

**Problem:** Python SSL verification error

**Solutions:**
1. The test automatically disables verification if CA cert not found
2. Copy `test_certs/ca-cert.pem` to test directory
3. Or manually set `context.verify_mode = ssl.CERT_NONE`

## CI/CD Integration

Add to your CI pipeline:

```yaml
# Example GitHub Actions
- name: Run NETCONF TLS tests
  run: |
    cd zephyr-workspace
    ./zephyr/scripts/twister \
      -p native_posix \
      -T modules/lib/clixon/zephyr/tests/netconf_tls \
      --inline-logs \
      --outdir twister-out
```

## Extending the Tests

To add new test cases:

1. Add test function to `pytest/test_netconf_tls.py`:
```python
def test_my_feature(dut, netconf_client):
    """Test description"""
    netconf_client.connect()
    netconf_client.start_tls()
    # Your test logic here
    assert condition
```

2. Use the `netconf_client` fixture for connection management

3. Run: `twister -T modules/lib/clixon/zephyr/tests/netconf_tls`

## Performance Benchmarks

Expected test execution times (native_posix):

- test_server_listening: ~2s
- test_tls_handshake: ~3s
- test_receive_hello: ~3s
- test_hello_exchange: ~4s
- test_tls_cipher_suite: ~3s
- test_multiple_connections: ~5s
- test_connection_timeout: ~8s

**Total:** ~30 seconds

## References

- [Twister Documentation](https://docs.zephyrproject.org/latest/develop/test/twister.html)
- [Pytest Documentation](https://docs.pytest.org/)
- [RFC 6241: NETCONF Protocol](https://www.rfc-editor.org/rfc/rfc6241.html)
- [RFC 7589: NETCONF over TLS](https://www.rfc-editor.org/rfc/rfc7589.html)
