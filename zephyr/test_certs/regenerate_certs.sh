#!/usr/bin/env pwsh
# Script to regenerate test certificates for NETCONF TLS
# Uses SHA-256 and X.509 v3 format

set -e

# Clean up old files
rm -f ca-key.pem ca-cert.pem ca-cert.srl
rm -f server-key.pem server.csr server-cert.pem
rm -f server-cert.der server-key.der
rm -f server_cert.h server_key.h

# Generate CA private key
openssl genrsa -out ca-key.pem 2048

# Generate CA certificate (v3, SHA-256)
openssl req -new -x509 -days 3650 -key ca-key.pem -out ca-cert.pem \
    -subj "/C=US/ST=Test/L=Test/O=Clixon Test CA/CN=Test CA" \
    -sha256

# Generate server private key
openssl genrsa -out server-key.pem 2048

# Generate server certificate signing request
openssl req -new -key server-key.pem -out server.csr \
    -subj "/C=US/ST=Test/L=Test/O=Clixon/CN=netconf-server"

# Create v3 extensions file
cat > v3.ext << EOF
basicConstraints=CA:FALSE
keyUsage=digitalSignature,keyEncipherment
extendedKeyUsage=serverAuth
subjectAltName=DNS:netconf-server,DNS:localhost,IP:192.168.1.100
EOF

# Sign server certificate with CA (v3, SHA-256)
openssl x509 -req -in server.csr -CA ca-cert.pem -CAkey ca-key.pem \
    -CAcreateserial -out server-cert.pem -days 3650 \
    -sha256 -extfile v3.ext

# Convert to DER format
openssl x509 -in server-cert.pem -outform DER -out server-cert.der
openssl rsa -in server-key.pem -outform DER -out server-key.der

# Convert DER to C header files
python der_to_c.py server-cert.der server_cert.h server_cert_der
python der_to_c.py server-key.der server_key.h server_key_der

# Clean up temporary files
rm -f v3.ext

echo "Certificates regenerated successfully!"
echo "Certificate info:"
openssl x509 -in server-cert.pem -text -noout | grep -E "Version|Signature Algorithm|Subject:" | head -5
