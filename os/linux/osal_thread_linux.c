/**
 * @file osal_thread_linux.c
 * @brief OS abstraction layer for threads on Linux.
 *
 * Provides thread creation, joining, and destruction using POSIX threads.
 *
 * @author Aravinthraj Ganesan
 */

#define _GNU_SOURCE


#include "osal_thread.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>

// Thread structure wrapping POSIX thread
struct osal_thread
{
    pthread_t thread_id;
};

/**
 * @brief Creates a new thread.
 *
 * Allocates and initializes a new thread using POSIX threads.
 *
 * @param out_thread Pointer that will receive the new thread object.
 * @param entry_fn Thread entry function.
 * @param entry_arg Argument passed to the thread function.
 * @param thread_name Optional thread name (Linux only).
 * @return 0 on success, negative value on error.
 */

int osal_thread_create(osal_thread_t ** out_thread, osal_thread_fn_t entry_fn, void* entry_arg, const char* thread_name)
{
    // Validate input parameters
    if(out_thread == NULL || entry_fn == NULL)
    {
        return -1;
    }

    // Allocate memory for thread structure
    osal_thread_t* thread = (osal_thread_t*) calloc (1, sizeof(*thread));

    if(thread == NULL)
        return -2;

    // Create the POSIX thread
    int result = pthread_create(&thread->thread_id, NULL, entry_fn, entry_arg);

    if(result!=0)
    {
        // Free memory if thread creation failed
        free(thread);
        return -3;
    }

    // Set thread name if supported on Linux
    #if defined(__linux__)

        // Check if thread name is provided
        if(thread_name && thread_name[0] != '\0')
        {
            // Truncate name to 16 bytes as required by Linux
            char trunc_thread_name[16];
            strncpy(trunc_thread_name, thread_name, sizeof(thread_name)-1);
            trunc_thread_name[sizeof(trunc_thread_name)-1] = '\0';
            pthread_setname_np(thread->thread_id, trunc_thread_name);
        }

    #endif 

    *out_thread = thread;

    // Return success
    return 0;
}

/**
 * @brief Joins a thread.
 *
 * Waits for the thread to terminate.
 *
 * @param thread Thread to join.
 * @return 0 on success, negative value on error.
 */
int osal_thread_join(osal_thread_t* thread)
{
    // Validate thread pointer
    if(thread == NULL)
    {
        return -1;
    }

    return pthread_join(thread->thread_id, NULL);
}

/**
 * @brief Destroys a thread.
 *
 * Frees the thread resources.
 *
 * @param thread Thread to destroy.
 */
void osal_thread_destroy(osal_thread_t* thread)
{
    // Free the thread memory
    free(thread);
}