# Testing NETCONF over TLS

## Overview

The Clixon NETCONF server now supports TLS encryption using mbedTLS. Self-signed test certificates are embedded for development and testing.

## Current Status

✅ **Implemented:**
- mbedTLS integration with entropy and CTR_DRBG
- TLS 1.2 server configuration
- Embedded self-signed test certificates (811 bytes cert + 1193 bytes key)
- Per-client TLS handshake
- TLS context cleanup on shutdown

## Build Configuration

Enable TLS in your `prj.conf`:
```kconfig
CONFIG_CLIXON_NETCONF=y
CONFIG_CLIXON_NETCONF_TLS=y
CONFIG_CLIXON_NETCONF_PORT=830
```

Memory impact: ~5.5 KB additional RAM for TLS support (from 517 KB to 523 KB)

## Test Certificates

### Location
- Generated in `zephyr/test_certs/`
- Embedded as C arrays in `server_cert.h` and `server_key.h`
- Self-signed CA with 10-year validity

### Certificate Details
- **CA Subject:** `/C=US/ST=Test/L=Test/O=Clixon Test CA/CN=Test CA`
- **Server Subject:** `/C=US/ST=Test/L=Test/O=Clixon/CN=netconf-server`
- **Validity:** 10 years (3650 days)
- **Key Size:** RSA 2048-bit

### Regenerating Certificates

Use the provided scripts:

**Windows (PowerShell):**
```powershell
cd modules/lib/clixon/zephyr
# Generate certificates
.\generate_test_certs.ps1

# Or manually:
cd test_certs
openssl genrsa -out ca-key.pem 2048
openssl req -new -x509 -days 3650 -key ca-key.pem -out ca-cert.pem ...
# (see generate_test_certs.sh for full commands)

# Convert to C arrays
cd ..
python der_to_c.py
```

**Linux/macOS:**
```bash
cd modules/lib/clixon/zephyr
bash generate_test_certs.sh
```

## Testing TLS Connection

### 1. Build and Run

```bash
cd $WEST_TOPDIR
west build -b qemu_x86 modules/lib/clixon/zephyr/sample
west build -t run
```

### 2. Test with OpenSSL s_client

In another terminal:
```bash
# Assuming QEMU networking is configured
openssl s_client -connect 192.168.1.1:830 \
    -CAfile modules/lib/clixon/zephyr/test_certs/ca-cert.pem
```

Expected output:
```
CONNECTED(00000003)
depth=1 C = US, ST = Test, L = Test, O = Clixon Test CA, CN = Test CA
verify return:1
depth=0 C = US, ST = Test, L = Test, O = Clixon, CN = netconf-server
verify return:1
---
Certificate chain
 0 s:C = US, ST = Test, L = Test, O = Clixon, CN = netconf-server
   i:C = US, ST = Test, L = Test, O = Clixon Test CA, CN = Test CA
---
...
<?xml version="1.0" encoding="UTF-8"?>
<hello xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
  <capabilities>
    <capability>urn:ietf:params:netconf:base:1.0</capability>
    <capability>urn:ietf:params:netconf:base:1.1</capability>
  </capabilities>
  <session-id>1</session-id>
</hello>
]]>]]>
```

### 3. Test with Python ncclient

```python
from ncclient import manager

# Connect to NETCONF server
m = manager.connect(
    host='192.168.1.1',
    port=830,
    username='admin',  # Not yet implemented
    password='admin',  # Not yet implemented
    hostkey_verify=False,
    device_params={'name': 'default'},
    timeout=30
)

# Receive server hello
print("Connected! Server capabilities:")
for cap in m.server_capabilities:
    print(f"  - {cap}")

# Close connection
m.close_session()
```

## Network Configuration for QEMU

### Native POSIX Build (Recommended for Testing)

```bash
# Build for native_posix with networking
west build -b native_posix modules/lib/clixon/zephyr/sample

# Run with networking
sudo -E west build -t run

# In another terminal, test connection
openssl s_client -connect 127.0.0.1:830 \
    -CAfile modules/lib/clixon/zephyr/test_certs/ca-cert.pem
```

### QEMU x86 with TAP/TUN

Add to `prj.conf`:
```kconfig
CONFIG_NET_CONFIG_SETTINGS=y
CONFIG_NET_CONFIG_MY_IPV4_ADDR="192.168.1.10"
CONFIG_NET_CONFIG_PEER_IPV4_ADDR="192.168.1.1"
```

Run with networking:
```bash
# Setup TAP interface (Linux)
sudo ip tuntap add dev tap0 mode tap user $USER
sudo ip addr add 192.168.1.1/24 dev tap0
sudo ip link set dev tap0 up

# Run QEMU
west build -t run

# Test from host
openssl s_client -connect 192.168.1.10:830 \
    -CAfile modules/lib/clixon/zephyr/test_certs/ca-cert.pem
```

## Security Warnings

⚠️ **DO NOT USE IN PRODUCTION:**
- Self-signed certificates
- No certificate validation
- No client authentication
- Test CA private key is committed to repo
- No certificate revocation checking

For production use:
1. Generate proper certificates from trusted CA
2. Enable client certificate verification (`MBEDTLS_SSL_VERIFY_REQUIRED`)
3. Store private keys securely (not in code)
4. Implement certificate rotation
5. Enable TLS 1.3 if supported
6. Configure appropriate cipher suites

## Troubleshooting

### TLS Handshake Fails

Check logs for:
```
[00:00:00.000,000] <err> clixon_service: TLS handshake failed: -0xXXXX
```

Common error codes:
- `-0x7780`: No valid certificate chain (cert not loaded)
- `-0x7200`: Connection reset by peer
- `-0x6900`: Invalid record MAC
- `-0x2700`: Memory allocation failed

### Certificate Not Found

Error:
```
fatal error: test_certs/server_cert.h: No such file or directory
```

Solution: Regenerate certificates with `python der_to_c.py`

### Out of Memory

Increase heap size in `prj.conf`:
```kconfig
CONFIG_HEAP_MEM_POOL_SIZE=131072  # 128 KB
```

Each TLS connection uses ~12-15 KB RAM.

## Implementation Details

### TLS Context Initialization (tls_init)

1. Initialize mbedTLS structures (ssl_config, x509_crt, pk_context, entropy, ctr_drbg)
2. Seed random number generator with entropy source
3. Parse embedded DER certificate and private key
4. Configure SSL defaults (TLS 1.2, stream transport)
5. Set RNG callback
6. Configure server certificate and key
7. Set auth mode to NONE (no client cert required)

### Per-Client Handshake (netconf_server_thread)

1. Accept TCP connection
2. Create `mbedtls_ssl_context` for client
3. Setup SSL with shared configuration
4. Set BIO callbacks (send/recv using BSD sockets)
5. Perform TLS handshake (loop until complete or error)
6. Send NETCONF hello over encrypted channel
7. Close TLS session with `ssl_close_notify`
8. Free SSL context and close socket

### Cleanup (tls_cleanup)

Called during `netconf_shutdown()` to free all mbedTLS resources.

## Future Enhancements

- [ ] Client certificate authentication (mTLS)
- [ ] Session resumption
- [ ] Runtime certificate provisioning via TLS credentials API
- [ ] Cipher suite configuration via Kconfig
- [ ] TLS 1.3 support
- [ ] Hardware crypto acceleration
- [ ] Certificate revocation (CRL/OCSP)
- [ ] PSK mode support

## References

- [RFC 7589: NETCONF over TLS](https://www.rfc-editor.org/rfc/rfc7589.html)
- [mbedTLS Documentation](https://mbed-tls.readthedocs.io/)
- [Zephyr TLS Credentials](https://docs.zephyrproject.org/latest/connectivity/networking/api/tls_credentials.html)
- [TLS_IMPLEMENTATION_GUIDE.md](TLS_IMPLEMENTATION_GUIDE.md) - Detailed implementation guide
