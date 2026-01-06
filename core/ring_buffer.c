/**
 * @file ring_buffer.c
 * @brief Telemetry ring buffer implementation.
 *
 * Thread-safe ring buffer for telemetry events.
 *
 * @author Aravinthraj Ganesan
 */


#include "ring_buffer.h"
#include <stdatomic.h>


// Struct declaration

typedef struct ring_buffer_s{
    telemetry_event_t* buffer;

    size_t capacity;
    size_t allocation;

    atomic_size_t head;
    atomic_size_t tail;

    atomic_uint_fast64_t dropped;
};

// Local function definitions

/**
 * @brief Calculates the next index in the ring buffer.
 *
 * Wraps around when reaching the allocation size.
 *
 * @param rb Ring buffer instance.
 * @param index Current index.
 * @return Next index.
 */

static inline size_t next_index(ring_buffer_t* rb, size_t index)
{
    index++;

    // When it reaches the maximum buffer size then wrap around
    if(index >= rb->allocation)
    {
        index = 0;
    }

    return index;
}

// Global function definitions

/**
 * @brief Initializes a ring buffer.
 *
 * Allocates memory and sets up atomic variables.
 *
 * @param rb Ring buffer instance.
 * @param capacity Buffer capacity.
 * @return true on success, false on failure.
 */
bool ring_buffer_init(ring_buffer_t** out_rb, size_t capacity)
{
    // Check the out_rb is not NULL and capacity not equal to 0
    if(out_rb == NULL || capacity == 0)
    {
        return false;
    }

    ring_buffer_t* rb = (ring_buffer_t*)calloc(1,sizeof(*rb));

    if(rb == NULL)
    {
        return false;
    }

    rb->capacity = capacity;            // No.of slots used
    rb->allocation = capacity + 1;      // No.of slots + 1 to distinguish between the Empty and Full

    // Allocate the buffer based on the capacity
    rb->buffer = (telemetry_event_t*)calloc(rb->allocation, sizeof(telemetry_event_t));

    // check if the memory allocation is successfull or not
    if(rb->buffer == NULL)
    {
        // if the memory allocation fails, reset the capacity and allocation on the ring buffer
        rb->capacity = 0;
        rb->allocation = 0;
        free(rb);
        return false;
    }

    // Initialize the atomic variables to 0
    atomic_store_explicit(&rb->head, 0, memory_order_relaxed);
    atomic_store_explicit(&rb->tail, 0, memory_order_relaxed);
    atomic_store_explicit(&rb->dropped, 0, memory_order_relaxed);

    *out_rb = rb;

    return true;
}


/**
 * @brief Frees the ring buffer.
 *
 * Deallocates memory and resets variables.
 *
 * @param rb Ring buffer instance.
 */
void ring_buffer_free(ring_buffer_t* rb)
{
    // Check the Null ptr
    if(rb == NULL)
        return;
    
    // Free the ring buffer
    free(rb->buffer);
    rb->buffer = NULL;

    // Initialize the variables to 0
    rb->capacity = 0x00;
    rb->allocation = 0x00;

    atomic_store_explicit(&rb->head, 0x00, memory_order_relaxed);
    atomic_store_explicit(&rb->tail, 0x00, memory_order_relaxed);
    atomic_store_explicit(&rb->dropped, 0x00, memory_order_relaxed);
}

/**
 * @brief Pushes an event to the ring buffer.
 *
 * Thread-safe push operation.
 *
 * @param rb Ring buffer instance.
 * @param event Event to push.
 * @return true on success, false on failure.
 */
bool ring_buffer_push(ring_buffer_t* rb, telemetry_event_t* event)
{
    if(rb == NULL || rb->buffer == NULL || event == NULL)
    {
        return false;
    }

    // Read the head (Produced owned) and tail (consumer owned)
    size_t head = atomic_load_explicit(&rb->head, memory_order_relaxed);
    size_t tail = atomic_load_explicit(&rb->tail, memory_order_acquire);

    size_t next = next_index(rb, head);

    // Check if the Ring buffer is full
    if(next == tail)
    {
        // if the ring buffer is full, then drop the event update and increment the droped count
        atomic_fetch_add_explicit(&rb->dropped, 0x01, memory_order_relaxed);
        return false;
    }

    // Copy the event to the new slot
    rb->buffer[head] = *event;

    // publish the new head
    atomic_store_explicit(&rb->head, next, memory_order_release);

    return true;

}

/**
 * @brief Pops an event from the ring buffer.
 *
 * Thread-safe pop operation.
 *
 * @param rb Ring buffer instance.
 * @param out Output event.
 * @return true on success, false on failure.
 */
bool ring_buffer_pop(ring_buffer_t* rb, telemetry_event_t* out)
{
    if(rb == NULL || rb->buffer == NULL || out == NULL)
    {
        return false;
    }

    // Read the head (Producer owned) and tail (consumer owned)
    size_t head = atomic_load_explicit(&rb->head, memory_order_acquire);
    size_t tail = atomic_load_explicit(&rb->tail, memory_order_relaxed);

    // Check if the ring buffer is empty
    if(head == tail)
    {
        return false;
    }

    // Read the event from the ring buffer
    *out = rb->buffer[tail];

    // calcualte next
    size_t next = next_index(rb, tail);

    // Publish the tail 
    atomic_store_explicit(&rb->tail, next, memory_order_release);

    return true;
}

/**
 * @brief Returns the number of events in the ring buffer.
 *
 * @param rb Ring buffer instance.
 * @return Number of events.
 */
size_t ring_buffer_count(const ring_buffer_t* rb)
{
    if(rb == NULL || rb->buffer == NULL)
        return 0;

    // Read the head and tail
    size_t head = atomic_load_explicit(&rb->head, memory_order_acquire);
    size_t tail = atomic_load_explicit(&rb->tail, memory_order_acquire);

    if(head >= tail)
        return head - tail;

    return (rb->allocation - tail) + head;
}

/**
 * @brief Returns the number of dropped events.
 *
 * @param rb Ring buffer instance.
 * @return Number of dropped events.
 */
uint64_t ring_buffer_dropped(const ring_buffer_t* rb)
{
    if(rb == NULL)
        return 0;
    
    return (atomic_load_explicit(&rb->dropped, memory_order_relaxed));
    
}
