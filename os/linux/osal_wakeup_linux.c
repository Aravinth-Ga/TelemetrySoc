/**
 * @file osal_wakeup_linux.c
 * @brief OS abstraction layer for wakeup mechanism on Linux.
 *
 * Provides event-based wakeup notifications using eventfd.
 *
 * @author Aravinthraj Ganesan
 */

#include "osal_wakeup.h"
#include <sys/eventfd.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

// Thread-safe wakeup structure using eventfd
struct osal_wakeup
{
    int event_fd;           // File descriptor for wakeup notifications
};

// Global function definitions

/**
 * @brief Creates a wakeup object.
 *
 * Allocates and initializes a new wakeup mechanism.
 *
 * @return Pointer to wakeup object, or NULL on failure.
 */
osal_wakeup_t* osal_wakeup_create(void)
{
    // Allocate memory for the wakeup structure
    osal_wakeup_t* wakeup = (osal_wakeup_t*) calloc(1, sizeof(*wakeup));

    if(wakeup == NULL)
        return NULL;

    /* Create a blocking eventfd
        - Initial counter value is 0
        - CLOEXEC avoids fd leaks across exec()
    */
    wakeup->event_fd = eventfd(0, EFD_CLOEXEC);

    if(wakeup->event_fd < 0)
    {
        free(wakeup);
        return NULL;
    }

    return wakeup;

}

/**
 * @brief Notifies the wakeup object.
 *
 * Sends a notification to wake up waiting threads.
 *
 * @param wakeup Wakeup object to notify.
 */
void osal_wakeup_notify(osal_wakeup_t* wakeup)
{
    // Validate wakeup pointer
    if(wakeup == NULL)
        return;

    const uint64_t notification_value = 1;
    ssize_t bytes_written = write(wakeup->event_fd, &notification_value, sizeof(notification_value));

    (void)bytes_written;

}

/**
 * @brief Waits for wakeup notification.
 *
 * Blocks until a notification is received.
 *
 * @param wakeup Wakeup object to wait on.
 */
void osal_wakeup_wait(osal_wakeup_t* wakeup)
{
    // Validate wakeup pointer
    if(wakeup == NULL)
        return;

    uint64_t accumulated_notification_count = 0;

    while(1)
    {
        ssize_t bytes_read = read(wakeup->event_fd, &accumulated_notification_count, sizeof(accumulated_notification_count));

        if(bytes_read == (ssize_t)sizeof(accumulated_notification_count))
        {
            // Successfully consumed the notifications
            return;
        }

        if(bytes_read < 0 && errno == EINTR)
        {
            // Interrupted by signal, retry
            continue;
        }

        // Return on any other errors
        return;
    }

}

/**
 * @brief Destroys a wakeup object.
 *
 * Cleans up resources and frees memory.
 *
 * @param wakeup Wakeup object to destroy.
 */
void osal_wakeup_destroy(osal_wakeup_t* wakeup)
{
    // Validate wakeup pointer
    if(wakeup == NULL)
        return;

    close(wakeup->event_fd);
    free(wakeup);
}

