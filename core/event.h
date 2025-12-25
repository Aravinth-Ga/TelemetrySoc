// Include guard to prevent multiple inclusions
#pragma once

#include "../api/type.h"
#include <stddef.h>         // size_t   
#include <stdbool.h>        // bool

// Ensure compatibility with C++ compilers
#ifdef __cplusplus
    extern "C" {
#endif

// Fixed payload size for telemetry events
#ifndef TELEMETRY_EVENT_PAYLOAD_MAX
    #define TELEMETRY_EVENT_PAYLOAD_MAX 128
#endif

// Telemetry event severity levels
typedef enum telemetry_level_e{
    TELEMETRY_LEVEL_DEBUG = 0,
    TELEMETRY_LEVEL_INFO,
    TELEMETRY_LEVEL_WARNING,
    TELEMETRY_LEVEL_ERROR
}telemetry_level_t;


// Telemetry event structure definition
typedef struct telemetry_event_s{
    uint32_t event_id;
    uint8_t level;
    uint8_t reserved;       // Padding for alignment
    uint16_t payload_size;
    uint64_t timestamp;
    uint8_t  payload[TELEMETRY_EVENT_PAYLOAD_MAX];
}telemetry_event_t;

// Function to create a telemetry event
bool telemetry_event_make(
    telemetry_event_t* event,
    uint32_t event_id,
    const void *payload,
    size_t payload_size,
    telemetry_level_t level
);

// utility function to get max payload size
static inline size_t telemetry_event_payload_max(void)
{
    return TELEMETRY_EVENT_PAYLOAD_MAX;
}


#ifdef __cplusplus
    }
#endif