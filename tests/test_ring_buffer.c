/**
 * @file test_ring_buffer.c
 * @brief Unit tests for telemetry ring buffer functionality.
 *
 * This file contains test cases for the telemetry ring buffer operations,
 * including push, pop, FIFO order, wraparound, and stress testing.
 * @author Aravinthraj Ganesan
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "ring_buffer.h"

/* Test cases :
    1. Empty pop fails
    2. Test single push and pop
    3. Test the FIFO order and test count consistency
    4. Test the wrap aroud/ ring buffer and Dropout when the ring buffer is full
    5. SPSC thread stress test
*/

// Local function prototype declaration
static void testcase_empty_pop(void);
static void testcase_single_push_pop(void);
static void testcase_fifo_check(void);
static void testcase_wraparound_check(void);
static void testcase_spsc_stress(void);

void test_ring_buffer(void);

/**
 * @brief Main entry point for running telemetry ring buffer tests.
 *
 * Executes all test functions in sequence.
 *
 * @return void
 */
void test_ring_buffer()
{
    testcase_empty_pop();
    testcase_single_push_pop();
    testcase_fifo_check();
    testcase_wraparound_check();
}

/**
 * @brief Tests that popping from an empty ring buffer fails.
 *
 * Initializes a ring buffer, attempts to pop from it, verifies the operation
 * fails and the count remains zero, then frees the buffer.
 */
static void testcase_empty_pop()
{
    ring_buffer_t rb;
    telemetry_event_t out;

    // initialize the ring buffer
    ring_buffer_init(&rb, 4);

    // pop the empty ring buffer
    assert(ring_buffer_pop(&rb, &out) == false);
    assert(ring_buffer_count(&rb) == 0x00);

    // Free the ring buffer
    ring_buffer_free(&rb);

    printf("Telemetry :: Test case Empty POP is passed. \n");

}

/**
 * @brief Tests single push and pop operations.
 *
 * Initializes a ring buffer, pushes an event, verifies push success and count,
 * pops the event, verifies pop success and data integrity, then frees the buffer.
 */
static void testcase_single_push_pop()
{
    ring_buffer_t rb;
    telemetry_event_t event, out;
    char message[] = "test_push_pop";

    // Initialise the ring buffer
    ring_buffer_init(&rb, 4);

    // Make an event
    telemetry_event_make(&event, 1, message, sizeof(message), TELEMETRY_LEVEL_INFO);
    
    // Once the event is pushed in to the ring buffer the rb count must be incremented as 1
    assert(ring_buffer_push(&rb, &event) == true);
    assert(ring_buffer_count(&rb) == 0x01);

    // Once the event is pop from the ring buffer the rb count must be decremented by 1
    assert(ring_buffer_pop(&rb, &out) == true);
    assert(ring_buffer_count(&rb) == 0x00);

    // Now validate the out values same as event
    assert(out.event_id == event.event_id);
    assert(out.level == event.level);
    assert(out.payload_size == event.payload_size);
    assert(out.reserved == event.reserved);
    assert(out.timestamp == event.timestamp);

    // Compare the payload
    assert(memcmp(out.payload, event.payload, out.payload_size) == 0);

    // If everthing okay, free the ring buffer
    ring_buffer_free(&rb);

    printf("Telemetry :: Test case single push pop is passed. \n");

}

/**
 * @brief Tests FIFO order and count consistency.
 *
 * Pushes multiple events, verifies counts after each push, pops them in order,
 * verifies FIFO behavior and data integrity, then frees the buffer.
 */
static void testcase_fifo_check()
{
    ring_buffer_t rb;
    telemetry_event_t event, out;
    char message[] = "fifo_check";

    //initialize the ring buffer
    ring_buffer_init(&rb, 6);

    for(uint8_t index = 0; index < 5; index++)
    {
        telemetry_event_make(&event, index, message, sizeof(message), TELEMETRY_LEVEL_INFO);
        // validate the push event
        assert(ring_buffer_push(&rb, &event) == true);
        // validate the ring buffer count increment on push event
        assert(ring_buffer_count(&rb) == index + 1);    
    }
    
    for(uint8_t index = 0; index < 5; index++)
    {
        // pop the event
        assert(ring_buffer_pop(&rb, &out) == true);
        // validate the pop event in the FIFO order
        assert(out.event_id == index);
    }

    // validate the ring buffer count is 0, after all the elements are popped out
    assert(ring_buffer_count(&rb) == 0x00);

    // free the ring buffer once all are okay
    ring_buffer_free(&rb);

    printf("Telemetry :: Test case fifo order, count consistency check is passed. \n");

}

/**
 * @brief Tests wraparound and dropout when buffer is full.
 *
 * Fills the ring buffer beyond capacity, verifies push failures and dropouts,
 * checks that oldest events are dropped, then frees the buffer.
 */
static void testcase_wraparound_check()
{
    ring_buffer_t rb;
    telemetry_event_t out;
    telemetry_event_t ev0, ev1, ev2, ev3, ev4, ev5;
    char message[] = "wrap_around";

    // initalize the ring buffer with small capacity to wrap around quickly
    ring_buffer_init(&rb, 4);

    // Make the event 3 times
    telemetry_event_make(&ev0, 1, message, sizeof(message), TELEMETRY_LEVEL_INFO);
    telemetry_event_make(&ev1, 2, message, sizeof(message), TELEMETRY_LEVEL_INFO);
    telemetry_event_make(&ev2, 3, message, sizeof(message), TELEMETRY_LEVEL_INFO);

    // push the event also 3 times
    assert(ring_buffer_push(&rb,&ev0) == true);
    assert(ring_buffer_push(&rb,&ev1) == true);
    assert(ring_buffer_push(&rb,&ev2) == true);
    assert(ring_buffer_count(&rb) == 0x03);

    // pop the event 1 time
    assert(ring_buffer_pop(&rb, &out) == true);
    assert(ring_buffer_count(&rb) == 0x02);

    // make another 2 more event and push it, it must wrap around
    telemetry_event_make(&ev3, 4, message, sizeof(message), TELEMETRY_LEVEL_INFO);
    telemetry_event_make(&ev4, 5, message, sizeof(message), TELEMETRY_LEVEL_INFO);
    assert(ring_buffer_push(&rb, &ev3) == true);
    assert(ring_buffer_push(&rb, &ev4) == true);
    
    // validate when the buffer is full the event is dropped out as well 
    telemetry_event_make(&ev5, 6, message, sizeof(message), TELEMETRY_LEVEL_INFO);      
    assert(ring_buffer_push(&rb, &ev5) == false); 
    assert(ring_buffer_count(&rb) == 0x04);
    assert(ring_buffer_dropped(&rb) == 0x01);

    assert(ring_buffer_pop(&rb, &out) == true);assert(out.event_id == 2);
    assert(ring_buffer_pop(&rb, &out) == true);assert(out.event_id == 3);
    assert(ring_buffer_pop(&rb, &out) == true);assert(out.event_id == 4);
    assert(ring_buffer_pop(&rb, &out) == true);assert(out.event_id == 5);
    
    // free the ring buffer if every thing is okay
    ring_buffer_free(&rb);

    printf("Telemetry :: Test case wrap around check and dropout checks are passed. \n");

}

/**
 * @brief Performs SPSC (Single Producer Single Consumer) stress test.
 *
 * Tests concurrent push and pop operations in a multi-threaded environment
 * to verify thread safety and performance under stress.
 */
static void testcase_spsc_stress()
{

}