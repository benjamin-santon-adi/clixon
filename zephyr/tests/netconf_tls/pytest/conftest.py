#!/usr/bin/env python3
"""
Pytest configuration and fixtures for NETCONF TLS tests
"""

import pytest
import logging

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)

@pytest.fixture(scope="module")
def test_netconf_tls(dut):
    """
    Module-level fixture that starts the NETCONF TLS test application
    
    The 'dut' (Device Under Test) fixture is provided by Twister's pytest plugin.
    It gives us access to the running Zephyr application.
    """
    logger = logging.getLogger(__name__)
    
    logger.info("=" * 70)
    logger.info("Starting NETCONF over TLS test suite")
    logger.info("=" * 70)
    
    # The application is already started by Twister
    # We just need to make sure it's ready
    
    # Wait for initialization messages
    dut.readlines_until(regex=".*Test ready.*", timeout=10)
    
    logger.info("NETCONF server initialized and ready")
    
    yield dut
    
    logger.info("=" * 70)
    logger.info("NETCONF TLS test suite completed")
    logger.info("=" * 70)
