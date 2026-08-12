#!/usr/bin/env python3
"""
NETCONF over TLS test using pytest and ncclient

This test validates that the Clixon NETCONF server:
1. Starts successfully with TLS enabled
2. Accepts TLS connections
3. Performs TLS handshake correctly
4. Sends valid NETCONF hello message
5. Responds to basic NETCONF operations
"""

import pytest
import socket
import ssl
import time
import logging
import os
from pathlib import Path

# Setup logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

# Test configuration
NETCONF_HOST = "192.0.2.1"
NETCONF_PORT = 830
CONNECTION_TIMEOUT = 10
HANDSHAKE_TIMEOUT = 5

# Get the certificate path relative to this test file
TEST_DIR = Path(__file__).parent.parent
CERT_DIR = TEST_DIR.parent.parent / "test_certs"
CA_CERT_PATH = CERT_DIR / "ca-cert.pem"


class NetconfTLSClient:
    """Simple NETCONF over TLS client for testing"""
    
    def __init__(self, host, port, ca_cert_path):
        self.host = host
        self.port = port
        self.ca_cert_path = ca_cert_path
        self.sock = None
        self.ssl_sock = None
        
    def connect(self, timeout=CONNECTION_TIMEOUT):
        """Establish TCP connection"""
        logger.info(f"Connecting to {self.host}:{self.port}")
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.settimeout(timeout)
        self.sock.connect((self.host, self.port))
        logger.info("TCP connection established")
        
    def start_tls(self, timeout=HANDSHAKE_TIMEOUT):
        """Perform TLS handshake"""
        logger.info("Starting TLS handshake")
        
        # Create SSL context
        context = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
        
        # Load CA certificate if it exists
        if self.ca_cert_path and os.path.exists(self.ca_cert_path):
            logger.info(f"Loading CA certificate from {self.ca_cert_path}")
            context.load_verify_locations(self.ca_cert_path)
            context.check_hostname = False  # Self-signed cert
            context.verify_mode = ssl.CERT_REQUIRED
        else:
            logger.warning("CA certificate not found, disabling verification")
            context.check_hostname = False
            context.verify_mode = ssl.CERT_NONE
        
        # Wrap socket with TLS
        self.ssl_sock = context.wrap_socket(
            self.sock, 
            server_hostname=self.host
        )
        
        logger.info("TLS handshake completed")
        logger.info(f"TLS version: {self.ssl_sock.version()}")
        logger.info(f"Cipher: {self.ssl_sock.cipher()}")
        
    def receive_hello(self, timeout=5):
        """Receive and parse NETCONF hello message"""
        logger.info("Waiting for NETCONF hello message")
        
        self.ssl_sock.settimeout(timeout)
        buffer = b""
        
        # NETCONF hello ends with ]]>]]>
        while b"]]>]]>" not in buffer:
            chunk = self.ssl_sock.recv(4096)
            if not chunk:
                raise ConnectionError("Connection closed by server")
            buffer += chunk
            
        hello_msg = buffer.decode('utf-8')
        logger.info(f"Received hello message ({len(hello_msg)} bytes)")
        logger.debug(f"Hello content:\n{hello_msg}")
        
        return hello_msg
        
    def send_hello(self):
        """Send NETCONF client hello"""
        client_hello = (
            '<?xml version="1.0" encoding="UTF-8"?>\n'
            '<hello xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">\n'
            '  <capabilities>\n'
            '    <capability>urn:ietf:params:netconf:base:1.0</capability>\n'
            '  </capabilities>\n'
            '</hello>\n'
            ']]>]]>\n'
        )
        
        logger.info("Sending client hello")
        self.ssl_sock.sendall(client_hello.encode('utf-8'))
        
    def close(self):
        """Close connection"""
        if self.ssl_sock:
            try:
                self.ssl_sock.close()
            except:
                pass
        if self.sock:
            try:
                self.sock.close()
            except:
                pass
        logger.info("Connection closed")


@pytest.fixture
def netconf_client():
    """Fixture that provides a NETCONF client"""
    client = NetconfTLSClient(NETCONF_HOST, NETCONF_PORT, CA_CERT_PATH)
    yield client
    client.close()


def test_server_listening(dut):
    """Test that the NETCONF server is listening on the specified port"""
    logger.info("=" * 60)
    logger.info("TEST: Server listening")
    logger.info("=" * 60)
    
    # Wait for server to start
    time.sleep(2)
    
    # Try to connect
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(5)
    
    try:
        sock.connect((NETCONF_HOST, NETCONF_PORT))
        logger.info(f"✓ Server is listening on {NETCONF_HOST}:{NETCONF_PORT}")
        sock.close()
    except Exception as e:
        pytest.fail(f"Server not listening: {e}")


def test_tls_handshake(dut, netconf_client):
    """Test TLS handshake with the server"""
    logger.info("=" * 60)
    logger.info("TEST: TLS handshake")
    logger.info("=" * 60)
    
    # Wait for server
    time.sleep(2)
    
    # Connect and perform TLS handshake
    netconf_client.connect()
    netconf_client.start_tls()
    
    # Verify TLS connection
    assert netconf_client.ssl_sock is not None
    logger.info(f"✓ TLS handshake successful")


def test_receive_hello(dut, netconf_client):
    """Test receiving NETCONF hello message"""
    logger.info("=" * 60)
    logger.info("TEST: Receive NETCONF hello")
    logger.info("=" * 60)
    
    # Wait for server
    time.sleep(2)
    
    # Connect with TLS
    netconf_client.connect()
    netconf_client.start_tls()
    
    # Receive hello
    hello = netconf_client.receive_hello()
    
    # Validate hello message
    assert '<?xml version="1.0"' in hello
    assert '<hello xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">' in hello
    assert '<capabilities>' in hello
    assert '<capability>urn:ietf:params:netconf:base:1.0</capability>' in hello
    assert '</hello>' in hello
    assert ']]>]]>' in hello
    
    logger.info("✓ Valid NETCONF hello received")


def test_hello_exchange(dut, netconf_client):
    """Test complete hello exchange (send and receive)"""
    logger.info("=" * 60)
    logger.info("TEST: NETCONF hello exchange")
    logger.info("=" * 60)
    
    # Wait for server
    time.sleep(2)
    
    # Connect with TLS
    netconf_client.connect()
    netconf_client.start_tls()
    
    # Receive server hello
    server_hello = netconf_client.receive_hello()
    assert '<hello xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">' in server_hello
    logger.info("✓ Server hello received")
    
    # Send client hello
    netconf_client.send_hello()
    logger.info("✓ Client hello sent")
    
    # Give server time to process
    time.sleep(1)
    
    logger.info("✓ Hello exchange completed")


def test_tls_cipher_suite(dut, netconf_client):
    """Test that a secure cipher suite is used"""
    logger.info("=" * 60)
    logger.info("TEST: TLS cipher suite")
    logger.info("=" * 60)
    
    # Wait for server
    time.sleep(2)
    
    # Connect with TLS
    netconf_client.connect()
    netconf_client.start_tls()
    
    # Get cipher information
    cipher = netconf_client.ssl_sock.cipher()
    cipher_name = cipher[0] if cipher else "Unknown"
    tls_version = netconf_client.ssl_sock.version()
    
    logger.info(f"TLS version: {tls_version}")
    logger.info(f"Cipher suite: {cipher_name}")
    
    # Verify TLS 1.2 or higher
    assert tls_version in ["TLSv1.2", "TLSv1.3"], f"Weak TLS version: {tls_version}"
    
    # Verify cipher is not weak
    weak_ciphers = ["DES", "RC4", "MD5", "NULL", "anon"]
    for weak in weak_ciphers:
        assert weak not in cipher_name, f"Weak cipher detected: {cipher_name}"
    
    logger.info("✓ Secure TLS configuration confirmed")


def test_multiple_connections(dut):
    """Test that server can handle multiple sequential connections"""
    logger.info("=" * 60)
    logger.info("TEST: Multiple connections")
    logger.info("=" * 60)
    
    # Wait for server
    time.sleep(2)
    
    num_connections = 3
    
    for i in range(num_connections):
        logger.info(f"Connection {i+1}/{num_connections}")
        
        client = NetconfTLSClient(NETCONF_HOST, NETCONF_PORT, CA_CERT_PATH)
        try:
            client.connect()
            client.start_tls()
            hello = client.receive_hello()
            assert '<hello' in hello
            logger.info(f"  ✓ Connection {i+1} successful")
        finally:
            client.close()
        
        # Small delay between connections
        time.sleep(0.5)
    
    logger.info(f"✓ All {num_connections} connections succeeded")


def test_connection_timeout(dut):
    """Test connection timeout behavior"""
    logger.info("=" * 60)
    logger.info("TEST: Connection timeout")
    logger.info("=" * 60)
    
    # Wait for server
    time.sleep(2)
    
    client = NetconfTLSClient(NETCONF_HOST, NETCONF_PORT, CA_CERT_PATH)
    
    try:
        # Connect and do TLS handshake
        client.connect()
        client.start_tls()
        
        # Receive hello
        hello = client.receive_hello()
        assert '<hello' in hello
        
        # Keep connection idle (server should eventually close or we maintain it)
        logger.info("Keeping connection idle for 5 seconds...")
        time.sleep(5)
        
        logger.info("✓ Connection timeout test completed")
        
    finally:
        client.close()


# Pytest hooks for better output
def pytest_configure(config):
    """Configure pytest"""
    config.addinivalue_line(
        "markers", "slow: marks tests as slow (deselect with '-m \"not slow\"')"
    )


def pytest_collection_modifyitems(config, items):
    """Modify test collection"""
    for item in items:
        # Add custom markers if needed
        pass
