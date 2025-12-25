/**
 * @file event.c
 * @brief Telemetry event implementation.
 *
 * Event creation and monotonic timestamp handling.
 *
 * @author Aravinthraj Ganesan
 */


#include "event.h"
#include <stdbool.h>
#include <string.h>
#include <time.h>


// Local function prototype declaration
static uint64_t telemetry_now_monotonic_ns(void);


// Function definitions

/**
 * @brief Initializes a telemetry event.
 *
 * Fills a telemetry_event_t with the given event ID, payload, severity level,
 * and assigns a monotonic timestamp.
 *
 * @param event      Event structure to initialize.
 * @param event_id   Unique identifier for the event.
 * @param payload    Payload data (may be NULL if payload_size is 0).
 * @param payload_size Size of the payload in bytes.
 * @param level      Event severity level.
 * @return true on success, false on failure.
 */

bool telemetry_event_make(telemetry_event_t * event, 
    uint32_t event_id, const void* payload, size_t payload_size, telemetry_level_t level)
{
    
    // Check for NULL pointer
    if(event == NULL)
    {
        return false;
    }

    // Check if payload size exceeds the maximum allowed
    if(payload_size > (size_t)TELEMETRY_EVENT_PAYLOAD_MAX )
    {
        return false;
    }

    // Initialize event fields
    event->event_id = event_id;
    event->level = level;
    event->reserved = 0x00;  // Reserved for future use
    event->payload_size = (uint16_t) payload_size;

    // Set the timestamp
    event->timestamp = telemetry_now_monotonic_ns();

    // Copy the payload to the struct
    if(payload_size > 0)
    {
        if(payload != NULL)
        {
            memcpy(event->payload, payload, payload_size);
        }
        else
        {
            // Payload size > 0 still payload is NULL, which is invalid
            return false;
        }
    }

    return true;
}

/**
 * @brief Returns the current monotonic time in nanoseconds.
 *
 * Uses the monotonic clock, which is not affected by system time changes.
 * The value represents nanoseconds since an unspecified starting point.
 *
 * @return Monotonic time in nanoseconds.
 */
static uint64_t telemetry_now_monotonic_ns(void)
{
    struct timespec ts;

    // Get the monotonic time in nanoseconds. This provides full seconds value and nanosecond precision.
    clock_gettime(CLOCK_MONOTONIC, &ts);

    // Convert seconds to nanoseconds, then add the nanosecond fraction to the final value.
    return ((uint64_t) ts.tv_sec * 1000000000ull) + ((uint64_t)ts.tv_nsec);

}