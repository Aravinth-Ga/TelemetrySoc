/**
 * @file event.c
 * @brief Telemetry event implementation.
 *
 * Event creation and monotonic timestamp handling.
 *
 * @author Aravinthraj Ganesan
 */


#include "event.h"
#include "osal_time.h"
#include <stdbool.h>
#include <string.h>
#include <time.h>


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
    event->timestamp = osal_telemetry_now_monotonic_ns();

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
