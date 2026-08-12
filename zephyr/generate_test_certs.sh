#!/bin/bash
# Generate test certificates for NETCONF TLS testing
# DO NOT USE IN PRODUCTION - self-signed certificates for testing only

set -e

CERT_DIR="test_certs"
mkdir -p $CERT_DIR

echo "Generating CA key and certificate..."
openssl genrsa -out $CERT_DIR/ca-key.pem 2048
openssl req -new -x509 -days 3650 -key $CERT_DIR/ca-key.pem \
    -out $CERT_DIR/ca-cert.pem \
    -subj "/C=US/ST=Test/L=Test/O=Clixon Test CA/CN=Test CA"

echo "Generating server key and certificate..."
openssl genrsa -out $CERT_DIR/server-key.pem 2048
openssl req -new -key $CERT_DIR/server-key.pem \
    -out $CERT_DIR/server.csr \
    -subj "/C=US/ST=Test/L=Test/O=Clixon/CN=netconf-server"

# Sign server certificate with CA
openssl x509 -req -days 3650 \
    -in $CERT_DIR/server.csr \
    -CA $CERT_DIR/ca-cert.pem \
    -CAkey $CERT_DIR/ca-key.pem \
    -CAcreateserial \
    -out $CERT_DIR/server-cert.pem

# Convert to DER format for embedding
echo "Converting to DER format..."
openssl x509 -in $CERT_DIR/server-cert.pem -outform DER -out $CERT_DIR/server-cert.der
openssl rsa -in $CERT_DIR/server-key.pem -outform DER -out $CERT_DIR/server-key.der

# Generate C array header file
echo "Generating C header file..."
xxd -i $CERT_DIR/server-cert.der > $CERT_DIR/server_cert.h
xxd -i $CERT_DIR/server-key.der > $CERT_DIR/server_key.h

# Rename arrays to match our code
sed -i 's/unsigned char .*_server_cert_der/static const unsigned char server_cert_der/' $CERT_DIR/server_cert.h
sed -i 's/unsigned int .*_server_cert_der_len/static const unsigned int server_cert_der_len/' $CERT_DIR/server_cert.h

sed -i 's/unsigned char .*_server_key_der/static const unsigned char server_key_der/' $CERT_DIR/server_key.h
sed -i 's/unsigned int .*_server_key_der_len/static const unsigned int server_key_der_len/' $CERT_DIR/server_key.h

echo "Certificate generation complete!"
echo ""
echo "Files generated in $CERT_DIR/:"
echo "  - ca-cert.pem (CA certificate for clients)"
echo "  - server-cert.pem (Server certificate)"
echo "  - server-key.pem (Server private key)"
echo "  - server_cert.h (C header for embedding)"
echo "  - server_key.h (C header for embedding)"
echo ""
echo "To test with OpenSSL client:"
echo "  openssl s_client -connect <ip>:830 -CAfile $CERT_DIR/ca-cert.pem"
