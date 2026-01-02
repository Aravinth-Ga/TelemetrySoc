#pragma once

#include <stdint.h>

/**
 * @file osal_time.h
 * @brief OS abstraction layer for time functions.
 *
 * Provides time management functions for cross-platform compatibility.
 *
 * @author Aravinthraj Ganesan
 */

#ifdef __cplusplus
    extern "C" {
#endif

uint64_t osal_telemetry_now_monotonic_ns(void);

#ifdef __cplusplus
    }
#endif