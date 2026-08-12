# STM32 Platform Limitation

## Issue

The Clixon library cannot be built for STM32-based boards (e.g., `nucleo_f767zi`) due to an enum name conflict between the STM32 HAL and Clixon library.

## Root Cause

1. **STM32 HAL**: The file `stm32xxx.h` (e.g., `stm32f7xx.h`) defines an anonymous enum:
   ```c
   typedef enum {
       RESET = 0,
       SET = !RESET
   } FlagStatus;
   ```
   This is defined at line 158 in `stm32f7xx.h`.

2. **Clixon Library**: The file `clixon_netconf_lib.h` defines:
   ```c
   enum test_option {
       SET,
       TEST_THEN_SET,
       TEST_ONLY
   };
   ```
   This is defined at line 130 in `clixon_netconf_lib.h`.

3. **Include Order**: When building for STM32:
   - `kernel.h` is included first (required for Zephyr)
   - `kernel.h` → ... → `cmsis_core_m.h` → `soc.h` → `stm32f7xx.h` (STM32 HAL)
   - Later, Clixon headers include `clixon_netconf_lib.h`
   - Result: Enum `SET` is declared twice, causing compilation error

4. **Workarounds Attempted**:
   - `#undef SET` after including STM32 headers: **Doesn't work** (enums cannot be undefined)
   - Compatibility headers: **Doesn't work** (STM32 headers already included via kernel.h)
   - Disabling NETCONF only: **Doesn't work** (core Clixon files like `clixon_xml.c` include NETCONF header unconditionally)

## Impact

- **Cannot build** Clixon samples or applications with `CONFIG_CLIXON=y` on STM32 platforms
- Affects all STM32 boards (F0, F1, F4, F7, H7, L4, etc.)
- Error message:
  ```
  clixon_netconf_lib.h:130:5: error: redeclaration of enumerator 'SET'
  ```

## Supported Platforms

The following platforms are verified to work:
- ✅ `qemu_x86` - QEMU x86 emulator
- ✅ `native_sim` - Native POSIX simulator
- ✅ `native_posix` - Legacy native POSIX
- ✅ `eval_adin6310t1lebz/max32690/m4` - ADI ADIN6310 field switch (MAX32690)

## Potential Solutions

To support STM32 platforms, one of the following upstream changes would be required:

### Option 1: Rename Clixon Enum (Recommended)
Modify `clixon_netconf_lib.h` to avoid common enum names:
```c
enum test_option {
    CLIXON_SET,           // was: SET
    CLIXON_TEST_THEN_SET, // was: TEST_THEN_SET
    CLIXON_TEST_ONLY      // was: TEST_ONLY
};
```

### Option 2: Use Enum Class (C++ Style)
Not applicable for C library.

### Option 3: Conditional Include
Move NETCONF types to a separate header that's only included when needed:
- Create `clixon_netconf_types.h` with enum definitions
- Only include in NETCONF-specific files
- Modify core files like `clixon_xml.c` to not include NETCONF headers unconditionally

### Option 4: Preprocessor Namespace
Add preprocessor guards in Clixon headers:
```c
#ifndef CLIXON_NETCONF_LIB_H_
#define CLIXON_NETCONF_LIB_H_

#ifdef SET
#define SET_BACKUP SET
#undef SET
#endif

enum test_option {
    SET,
    TEST_THEN_SET,
    TEST_ONLY
};

#ifdef SET_BACKUP
#define SET SET_BACKUP
#undef SET_BACKUP
#endif
```

## Recommendations

1. **For Clixon Users**: Use non-STM32 platforms for NETCONF applications
2. **For STM32 Users**: Consider alternative NETCONF implementations
3. **For Developers**: Coordinate with Clixon upstream to rename conflicting enums

## Status

- **Reported**: 2024 (this document)
- **Upstream Issue**: TBD
- **Workaround**: Use alternative hardware platforms

## Contact

For questions or to contribute fixes, contact:
- Clixon project: https://github.com/clicon/clixon
- Zephyr integration: https://github.com/benjamin-santon-adi/clixon (zephyr-module branch)
