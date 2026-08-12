/*
 * Copyright (c) 2024 Benjamin Santon ADI
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * STM32 compatibility workarounds for Clixon
 *
 * Handles namespace conflicts between STM32 HAL and Clixon library:
 * - SET enum defined in both stm32xxx.h and clixon_netconf_lib.h
 */

#ifndef _CLIXON_STM32_COMPAT_H
#define _CLIXON_STM32_COMPAT_H

#ifdef CONFIG_SOC_FAMILY_STM32

/* Save STM32 HAL SET definition if it exists */
#ifdef SET
#define STM32_HAL_SET SET
#undef SET
#endif

/* Save STM32 HAL RESET definition if it exists */
#ifdef RESET  
#define STM32_HAL_RESET RESET
#undef RESET
#endif

/* Note: After including Clixon headers, the Clixon enum values will be defined
 * STM32 HAL macros are typically only needed in driver code, not application code
 * If both are needed in the same file, use STM32_HAL_SET instead of SET
 */

#endif /* CONFIG_SOC_FAMILY_STM32 */

#endif /* _CLIXON_STM32_COMPAT_H */
