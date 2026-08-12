#!/usr/bin/env python3
"""Convert DER file to C header array."""
import sys

def der_to_c_header(der_file, h_file, array_name):
    with open(der_file, 'rb') as f:
        der_data = f.read()
    
    with open(h_file, 'w') as f:
        f.write(f'/* Auto-generated from {der_file} */\n')
        f.write(f'static const unsigned char {array_name}[] = {{\n')
        
        for i in range(0, len(der_data), 12):
            chunk = der_data[i:i+12]
            hex_bytes = ', '.join(f'0x{b:02x}' for b in chunk)
            f.write(f'    {hex_bytes},\n')
        
        f.write('};\n')
        f.write(f'static const unsigned int {array_name}_len = {len(der_data)};\n')
    
    print(f'Generated {h_file} with {len(der_data)} bytes')

if __name__ == '__main__':
    if len(sys.argv) != 4:
        print(f'Usage: {sys.argv[0]} <der_file> <h_file> <array_name>')
        sys.exit(1)
    
    der_to_c_header(sys.argv[1], sys.argv[2], sys.argv[3])
