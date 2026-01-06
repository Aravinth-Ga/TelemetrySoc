#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include "event.h"

#ifdef __cplusplus
    extern "C" {
#endif

typedef struct ring_buffer_s ring_buffer_t;


// global ring buffer functions
bool ring_buffer_init(ring_buffer_t** out_rb, size_t capacity);
void ring_buffer_free(ring_buffer_t* rb);

// Producer thread : push the event to the ring buffer
bool ring_buffer_push(ring_buffer_t* rb, telemetry_event_t* event);

// Consumer thread : pop the event from the ring buffer
bool ring_buffer_pop(ring_buffer_t* rb, telemetry_event_t* out);


// Helper functions
size_t ring_buffer_count(const ring_buffer_t* rb);
uint64_t ring_buffer_dropped(const ring_buffer_t* rb);



#ifdef __cplusplus
    }
#endif