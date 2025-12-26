#pragma once

#include <stddef.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdlib.h>
#include "event.h"

#ifdef __cplusplus
    extern "C" {
#endif


typedef struct ring_buffer_s{
    telemetry_event_t* buffer;

    size_t capacity;
    size_t allocation;

    _Atomic size_t head;
    _Atomic size_t tail;

    _Atomic uint64_t dropped;

}ring_buffer_t;


// global ring buffer functions
bool ring_buffer_init(ring_buffer_t* rb, size_t capacity);
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