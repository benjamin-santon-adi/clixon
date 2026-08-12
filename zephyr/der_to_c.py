#!/usr/bin/env python3
"""Convert DER certificate/key files to C array header files"""

import sys
from pathlib import Path

def der_to_c_array(der_file, array_name):
    """Convert DER file to C array definition"""
    with open(der_file, 'rb') as f:
        data = f.read()
    
    # Generate C array
    lines = [f"/* Auto-generated from {der_file.name} */\n"]
    lines.append(f"static const unsigned char {array_name}[] = {{\n")
    
    # Format as hex bytes, 12 per line
    for i in range(0, len(data), 12):
        chunk = data[i:i+12]
        hex_bytes = ', '.join(f'0x{b:02x}' for b in chunk)
        lines.append(f"    {hex_bytes},\n")
    
    lines.append("};\n")
    lines.append(f"static const unsigned int {array_name}_len = {len(data)};\n")
    
    return ''.join(lines)

def main():
    cert_dir = Path('test_certs')
    
    # Convert certificate
    cert_h = der_to_c_array(cert_dir / 'server-cert.der', 'server_cert_der')
    with open(cert_dir / 'server_cert.h', 'w') as f:
        f.write(cert_h)
    print(f"Generated {cert_dir / 'server_cert.h'}")
    
    # Convert key
    key_h = der_to_c_array(cert_dir / 'server-key.der', 'server_key_der')
    with open(cert_dir / 'server_key.h', 'w') as f:
        f.write(key_h)
    print(f"Generated {cert_dir / 'server_key.h'}")
    
    print("\nCertificate files ready for embedding!")
    print(f"Certificate size: {(cert_dir / 'server-cert.der').stat().st_size} bytes")
    print(f"Private key size: {(cert_dir / 'server-key.der').stat().st_size} bytes")

if __name__ == '__main__':
    main()
