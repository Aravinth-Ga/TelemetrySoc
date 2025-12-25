#include "ring_buffer.h"



// global function definition

// ring buffer initialization
bool ring_buffer_init(ring_buffer_t* rb, size_t capacity)
{
    if(rb == NULL)
    {
        return false;
    }

    if(capacity == 0x00)
    {
        return false;
    }

    // Inialize the values 
    rb->capacity = capacity;        // no of slots used
    rb->allocation = capacity + 1;  // increment the slot +1 to distinguish between the empty and full

    // Allocate the buffer
    rb->buffer = (telemetry_event_t*)calloc(rb->allocation, sizeof(telemetry_event_t));

    if(rb->buffer == NULL)
    {
        // if the memory allocation failed, then reset the capacity and allocation
        rb->capacity = 0;
        rb->allocation = 0;
        return false;
    }

    // Initalise the Atomic variables
    atomic_store_explicit(&rb->head, 0x00, memory_order_relaxed);
    atomic_store_explicit(&rb->tail, 0x00, memory_order_relaxed);
    atomic_store_explicit(&rb->dropped, 0x00, memory_order_relaxed);

    return true;
}
