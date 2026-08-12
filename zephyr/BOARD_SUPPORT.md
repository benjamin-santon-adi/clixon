# Clixon Board Support Status

## Overview

This document tracks the status of Clixon library support across different Zephyr RTOS platforms.

## Supported Platforms

The following platforms have been verified to build and run with Clixon:

### ✅ QEMU x86 (`qemu_x86`)
- **Status**: Fully supported
- **Features**: Full Clixon library, NETCONF, TLS
- **Memory**: 523,260 bytes RAM (with TLS)
- **Testing**: Primary development platform
- **Network**: Simulated Ethernet
- **Notes**: Reference implementation, all features enabled

### ✅ Native POSIX Simulator (`native_sim`, `native_posix`)
- **Status**: Fully supported  
- **Features**: Full Clixon library, NETCONF, TLS
- **Memory**: Similar to qemu_x86
- **Testing**: CI/CD friendly
- **Network**: Native TAP/TUN interface
- **Notes**: Best for development and automated testing

### ✅ ADI ADIN6310 Field Switch (`eval_adin6310t1lebz/max32690/m4`)
- **Status**: Fully supported
- **Features**: Full Clixon library, NETCONF, TLS
- **MCU**: Analog Devices MAX32690 (Cortex-M4F)
  - 1 MB RAM
  - 3 MB Flash
- **Ethernet**: Custom ADIN6310 field switch over SPI4
  - Compatible: `adi,eth-adinx310`
  - Supports multiple PHY types (ADIN1100, ADIN1300)
- **Console**: UART3 (lpuart0) at 115200 baud
- **Binary Size**: ~7.9 MB (.elf with debug symbols)
- **Heap**: 131 KB configured
- **Network Config**: 192.168.1.100/24
- **Notes**: Production-ready industrial Ethernet platform

## Unsupported Platforms

### ❌ STM32-based Boards (All Variants)

**Examples**: `nucleo_f767zi`, `nucleo_h743zi`, `stm32f4_disco`, etc.

**Status**: Not supported - compilation error

**Reason**: Enum name conflict between STM32 HAL and Clixon library

**Error Message**:
```
clixon_netconf_lib.h:130:5: error: redeclaration of enumerator 'SET'
stm32f7xx.h:158:3: note: previous definition of 'SET' with type 'enum <anonymous>'
```

**Technical Details**:
1. STM32 HAL headers define: `enum { RESET=0, SET=!RESET }`
2. Clixon library defines: `enum test_option { SET, TEST_THEN_SET, TEST_ONLY }`
3. STM32 headers are included via `kernel.h` before Clixon headers
4. C enum values cannot be undefined with preprocessor directives

**Affected Families**:
- STM32F0, F1, F2, F3, F4, F7
- STM32H7, L0, L1, L4, L5
- STM32G0, G4, U5
- STM32WB, WL

**Workarounds Attempted**:
- ✗ `#undef SET` after STM32 headers (doesn't work for enums)
- ✗ Compatibility headers (STM32 already included via kernel.h)
- ✗ Disabling NETCONF only (core files include NETCONF header unconditionally)
- ✗ Disabling full library (service requires library headers)

**See Also**: [STM32_LIMITATION.md](STM32_LIMITATION.md) for detailed analysis

## Platform Requirements

### Minimum Requirements
- **RAM**: 256 KB (basic), 512 KB (with TLS)
- **Flash**: 512 KB (basic), 1 MB (with TLS)  
- **Network**: Ethernet (hardware or simulated)
- **POSIX API**: Required for file operations
- **Threading**: Zephyr native threading support

### Recommended Configuration
- **Heap**: 131 KB (`CONFIG_HEAP_MEM_POOL_SIZE=131072`)
- **Network Buffers**: 
  - RX: 16 packets, 32 buffers
  - TX: 16 packets, 32 buffers
- **TCP Support**: `CONFIG_NET_TCP=y`
- **BSD Sockets**: `CONFIG_NET_SOCKETS=y`
- **TLS**: mbedTLS 2.x/3.x compatible

## Testing Status

### Automated Tests
- **QEMU x86**: ✅ Passing (7 test cases)
- **Native Sim**: ✅ Passing (7 test cases)
- **ADIN6310**: ⏳ Pending (hardware required)

### Test Cases
1. Server listening on port 830
2. TLS handshake
3. Receive NETCONF hello
4. Hello message exchange
5. TLS cipher suite validation
6. Multiple concurrent connections
7. Connection timeout handling

See [tests/netconf_tls/TEST_STATUS.md](tests/netconf_tls/TEST_STATUS.md) for details.

## Board Configuration Files

### Required Files per Board

1. **Device Tree Overlay**: `boards/<board>.overlay`
   - Configure Ethernet interface
   - Set console UART (if needed)
   - Enable required peripherals

2. **Kconfig Configuration**: `boards/<board>.conf`
   - Set heap size (131 KB minimum)
   - Configure network settings (IP, gateway, netmask)
   - Enable Ethernet driver
   - Adjust buffer counts

### Example: eval_adin6310t1lebz/max32690/m4

**Overlay** (`boards/eval_adin6310t1lebz_max32690_m4.overlay`):
```c
/* ADIN6310 Ethernet interface and console are already configured in board dtsi */
/* No additional overlay needed - ethernet node is under adinx310@0 */
/* Console is uart3 (lpuart0) */
```

**Config** (`boards/eval_adin6310t1lebz_max32690_m4.conf`):
```
CONFIG_HEAP_MEM_POOL_SIZE=131072
CONFIG_NET_CONFIG_SETTINGS=y
CONFIG_NET_CONFIG_MY_IPV4_ADDR="192.168.1.100"
CONFIG_NET_CONFIG_MY_IPV4_NETMASK="255.255.255.0"
CONFIG_NET_CONFIG_MY_IPV4_GW="192.168.1.1"
CONFIG_NET_L2_ETHERNET=y
CONFIG_NET_PKT_RX_COUNT=16
CONFIG_NET_PKT_TX_COUNT=16
CONFIG_NET_BUF_RX_COUNT=32
CONFIG_NET_BUF_TX_COUNT=32
CONFIG_SYSTEM_WORKQUEUE_STACK_SIZE=4096
```

## Adding New Board Support

To add support for a new board (non-STM32):

1. **Check Requirements**:
   - Verify board has sufficient RAM/Flash
   - Ensure Ethernet support exists
   - Confirm no conflicting enum definitions

2. **Create Board Files**:
   ```bash
   cd modules/lib/clixon/zephyr/sample/boards
   cp qemu_x86.overlay <your_board>.overlay
   cp qemu_x86.conf <your_board>.conf
   ```

3. **Update Files**:
   - Modify overlay for board-specific DTS nodes
   - Adjust config for memory constraints
   - Set correct network driver

4. **Update sample.yaml**:
   ```yaml
   platform_allow:
     - your_board
   ```

5. **Test Build**:
   ```bash
   west build -b your_board modules/lib/clixon/zephyr/sample
   ```

6. **Verify Runtime** (if hardware available):
   ```bash
   west flash
   # Connect to serial console
   # Verify "NETCONF server initialized" message
   ```

## Known Issues

1. **STM32 Enum Conflict**: See above, requires upstream fix in Clixon
2. **Memory Constraints**: Boards with <256 KB RAM cannot run full stack
3. **TLS Overhead**: Adds ~160 KB to binary, ~5 KB to RAM

## Future Work

- [ ] Investigate Nordic nRF5x series support
- [ ] Test with NXP i.MX RT series (non-STM32)
- [ ] Optimize memory usage for constrained devices
- [ ] Add support for IPv6-only builds
- [ ] Implement NETCONF over SSH (alternative to TLS)

## Contact

For board-specific issues:
- Zephyr integration: https://github.com/benjamin-santon-adi/clixon (zephyr-module branch)
- Upstream Clixon: https://github.com/clicon/clixon

Last Updated: 2024
