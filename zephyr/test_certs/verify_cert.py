import re

# Read header
header = open('server_cert.h').read()
match = re.search(r'server_cert_der\[\]\s*=\s*\{([^}]+)\}', header, re.DOTALL)
bytes_str = match.group(1)

# Parse hex bytes from header
bytes_list = []
for item in bytes_str.split(','):
    item = item.strip()
    if item and item.startswith('0x'):
        bytes_list.append(int(item, 16))

# Read DER file
der_data = open('server-cert.der', 'rb').read()

print(f'Header bytes: {len(bytes_list)}')
print(f'DER file bytes: {len(der_data)}')
print(f'Match: {bytes(bytes_list) == der_data}')

if bytes(bytes_list) != der_data:
    print('MISMATCH!')
    for i, (a, b) in enumerate(zip(bytes(bytes_list), der_data)):
        if a != b:
            print(f'First diff at byte {i}: header=0x{a:02x} file=0x{b:02x}')
            break
else:
    print('Files match perfectly!')
