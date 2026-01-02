#pragma once

#include <stdint.h>


#ifdef __cplusplus
    extern "C" {
#endif

// Wakeup handle type
typedef struct osal_wakeup osal_wakeup_t;

// Create a wakeup object, return NULL on failure
osal_wakeup_t* osal_wakeup_create(void);

// Notify the wakeup object to wake up waiting threads
void osal_wakeup_notify(osal_wakeup_t* wakeup);

// Wait until notified
void osal_wakeup_wait(osal_wakeup_t* wakeup);

// Destroy the wakeup object
void osal_wakeup_destroy(osal_wakeup_t* wakeup);






#ifdef __cplusplus
    }
#endif