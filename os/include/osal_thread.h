/**
 * @file osal_thread.h
 * @brief OS abstraction layer for threads.
 *
 * Provides thread management functions for cross-platform compatibility.
 *
 * @author Aravinthraj Ganesan
 */

#pragma once

#include <stddef.h>

#ifdef __cplusplus
    extern "C" {
#endif

// Thread handle type
typedef struct osal_thread osal_thread_t;
// Function pointer type for thread entry point
typedef void* (*osal_thread_fn_t)(void*);


// Creates a new thread.
int osal_thread_create(osal_thread_t ** out_thread, osal_thread_fn_t entry_fn, void* entry_arg, const char* thread_name);

// Waits for the thread to finish.
int osal_thread_join(osal_thread_t* thread);

// Cleans up thread resources.
void osal_thread_destroy(osal_thread_t* thread);

#ifdef __cplusplus
    }
#endif