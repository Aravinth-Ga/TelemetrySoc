#include "osal_time.h"
#include <time.h>



/**
 * @brief Returns the current monotonic time in nanoseconds.
 *
 * Uses the monotonic clock, which is not affected by system time changes.
 * The value represents nanoseconds since an unspecified starting point.
 *
 * @return Monotonic time in nanoseconds.
 */
uint64_t osal_telemetry_now_monotonic_ns(void)
{
    struct timespec ts;

    // Get the monotonic time in nanoseconds. This provides full seconds value and nanosecond precision.
    clock_gettime(CLOCK_MONOTONIC, &ts);

    // Convert seconds to nanoseconds, then add the nanosecond fraction to the final value.
    return ((uint64_t) ts.tv_sec * 1000000000ull) + ((uint64_t)ts.tv_nsec);

}