# NETCONF over TLS Implementation Guide

## Current Status

The NETCONF server in clixon service currently runs **without TLS encryption**. Basic TCP socket support is implemented and working.

## Implementation Plan for TLS Support

### 1. Prerequisites

When `CONFIG_CLIXON_NETCONF_TLS=y` is enabled, the following are automatically included:
- mbedTLS library
- TLS credentials subsystem
- Network socket TLS options

### 2. Required Changes to clixon_service.c

#### A. Add mbedTLS Headers
```c
#ifdef CONFIG_CLIXON_NETCONF_TLS
#include <mbedtls/ssl.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/x509_crt.h>
#include <mbedtls/pk.h>
#include <mbedtls/error.h>
#endif
```

#### B. Add TLS Context to Service State
```c
static struct {
    // ... existing fields ...
#ifdef CONFIG_CLIXON_NETCONF_TLS
    mbedtls_ssl_config ssl_conf;
    mbedtls_x509_crt server_cert;
    mbedtls_pk_context server_key;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    bool tls_initialized;
#endif
} clixon_service;
```

#### C. TLS Initialization Function
```c
static int tls_init(void)
{
    int ret;
    
    // Initialize mbedTLS structures
    mbedtls_ssl_config_init(&clixon_service.ssl_conf);
    mbedtls_x509_crt_init(&clixon_service.server_cert);
    mbedtls_pk_init(&clixon_service.server_key);
    mbedtls_entropy_init(&clixon_service.entropy);
    mbedtls_ctr_drbg_init(&clixon_service.ctr_drbg);
    
    // Seed RNG
    ret = mbedtls_ctr_drbg_seed(&clixon_service.ctr_drbg,
                                 mbedtls_entropy_func,
                                 &clixon_service.entropy,
                                 "netconf_server", 15);
    if (ret != 0) return -EINVAL;
    
    // Load certificate and key (TODO: from TLS credentials)
    // ret = mbedtls_x509_crt_parse(&clixon_service.server_cert, ...);
    // ret = mbedtls_pk_parse_key(&clixon_service.server_key, ...);
    
    // Configure SSL defaults
    ret = mbedtls_ssl_config_defaults(&clixon_service.ssl_conf,
                                       MBEDTLS_SSL_IS_SERVER,
                                       MBEDTLS_SSL_TRANSPORT_STREAM,
                                       MBEDTLS_SSL_PRESET_DEFAULT);
    if (ret != 0) return -EINVAL;
    
    // Set RNG
    mbedtls_ssl_conf_rng(&clixon_service.ssl_conf,
                         mbedtls_ctr_drbg_random,
                         &clixon_service.ctr_drbg);
    
    // Set certificates
    // mbedtls_ssl_conf_ca_chain(&clixon_service.ssl_conf, ...);
    // mbedtls_ssl_conf_own_cert(&clixon_service.ssl_conf, ...);
    
    clixon_service.tls_initialized = true;
    return 0;
}
```

#### D. Per-Client TLS Handshake
```c
// In netconf_server_thread(), after accept():
#ifdef CONFIG_CLIXON_NETCONF_TLS
    mbedtls_ssl_context ssl;
    mbedtls_ssl_init(&ssl);
    
    ret = mbedtls_ssl_setup(&ssl, &clixon_service.ssl_conf);
    if (ret != 0) {
        close(client_sock);
        continue;
    }
    
    mbedtls_ssl_set_bio(&ssl, &client_sock,
                        (mbedtls_ssl_send_t *)send,
                        (mbedtls_ssl_recv_t *)recv, NULL);
    
    // Perform handshake
    while ((ret = mbedtls_ssl_handshake(&ssl)) != 0) {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ &&
            ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
            mbedtls_ssl_free(&ssl);
            close(client_sock);
            goto next_client;
        }
    }
    
    // Use mbedtls_ssl_write() instead of send()
    mbedtls_ssl_write(&ssl, hello_msg, strlen(hello_msg));
    
    mbedtls_ssl_close_notify(&ssl);
    mbedtls_ssl_free(&ssl);
#endif
```

### 3. Certificate Management

Certificates must be provisioned before the NETCONF server starts. Two approaches:

#### A. Compile-Time Embedded Certificates
```c
// Define in separate file
static const char server_cert_pem[] = 
    "-----BEGIN CERTIFICATE-----\n"
    "...\n"
    "-----END CERTIFICATE-----\n";

static const char server_key_pem[] = 
    "-----BEGIN RSA PRIVATE KEY-----\n"
    "...\n"
    "-----END RSA PRIVATE KEY-----\n";

// Parse during tls_init():
mbedtls_x509_crt_parse(&server_cert, server_cert_pem, strlen(server_cert_pem) + 1);
mbedtls_pk_parse_key(&server_key, server_key_pem, strlen(server_key_pem) + 1, NULL, 0);
```

#### B. Runtime Provisioning via TLS Credentials API
```c
// Application calls before service init:
#include <zephyr/net/tls_credentials.h>

#define NETCONF_SERVER_CERT_TAG 1
#define NETCONF_SERVER_KEY_TAG 2

tls_credential_add(NETCONF_SERVER_CERT_TAG,
                   TLS_CREDENTIAL_SERVER_CERTIFICATE,
                   cert_der, cert_der_len);
                   
tls_credential_add(NETCONF_SERVER_KEY_TAG,
                   TLS_CREDENTIAL_PRIVATE_KEY,
                   key_der, key_der_len);
```

### 4. Build Configuration

CMakeLists.txt already updated to include mbedTLS headers:
```cmake
if(CONFIG_CLIXON_NETCONF_TLS)
  zephyr_include_directories(${ZEPHYR_CURRENT_MODULE_DIR}/../../crypto/mbedtls/include)
endif()
```

### 5. Testing TLS Connection

#### Generate Test Certificates:
```bash
# Generate CA key and certificate
openssl genrsa -out ca-key.pem 2048
openssl req -new -x509 -days 365 -key ca-key.pem -out ca-cert.pem

# Generate server key and CSR
openssl genrsa -out server-key.pem 2048
openssl req -new -key server-key.pem -out server.csr

# Sign server certificate with CA
openssl x509 -req -days 365 -in server.csr -CA ca-cert.pem \
  -CAkey ca-key.pem -CAcreateserial -out server-cert.pem
```

#### Test Connection:
```bash
# Using OpenSSL s_client
openssl s_client -connect <ip>:830 -CAfile ca-cert.pem

# Using ncclient (Python NETCONF client)
from ncclient import manager
m = manager.connect(host='<ip>', port=830, username='admin',
                    password='admin', hostkey_verify=False)
```

### 6. Current Limitations

- Certificate loading not yet implemented
- Need to integrate with Zephyr TLS credentials system
- Client certificate verification optional
- Session resumption not implemented
- Cipher suite selection uses defaults

### 7. Memory Impact

Expected additional RAM usage with TLS enabled:
- mbedTLS context: ~8-12 KB per connection
- Certificate storage: ~2-4 KB
- Total additional: ~15-20 KB per active session

### 8. Security Considerations

- Use TLS 1.2 or higher (TLS 1.3 supported in mbedTLS 3.x)
- Disable weak cipher suites
- Enable certificate verification for production
- Consider mutual TLS (mTLS) for client authentication
- Rotate certificates before expiry
- Store private keys securely (consider using secure element)

### 9. Future Enhancements

- [ ] Implement certificate loading from TLS credentials
- [ ] Add support for certificate chains
- [ ] Implement session resumption
- [ ] Add cipher suite configuration via Kconfig
- [ ] Support for hardware crypto acceleration
- [ ] Certificate revocation checking (CRL/OCSP)
- [ ] TLS 1.3 support (PSK mode)
- [ ] Client certificate authentication

### 10. References

- RFC 7589: NETCONF over TLS
- mbedTLS Documentation: https://tls.mbed.org/
- Zephyr TLS Guide: https://docs.zephyrproject.org/latest/connectivity/networking/api/tls_credentials.html
