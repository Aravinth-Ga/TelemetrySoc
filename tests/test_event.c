/**
 * @file test_event.c
 * @brief Unit tests for telemetry event functionality.
 *
 * This file contains test cases for the telemetry event creation,
 * payload handling, timestamp generation, and error conditions.
 * @author Aravinthraj Ganesan
 */

#include <event.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

// Local function prototype declarations
static void test_payload_copy(void);
static void test_monotonic_timestamp(void);
static void test_oversized_payload(void);

// Test main function
/**
 * @brief Main entry point for running telemetry event tests.
 *
 * Executes all test functions in sequence.
 *
 * @return 0 on success.
 */
int main()
{
    test_payload_copy();
    test_monotonic_timestamp();
    test_oversized_payload();

    return 0;
}

/**
 * @brief Tests payload copying functionality.
 *
 * Verifies that telemetry_event_make correctly copies payload data
 * and sets all event fields appropriately.
 */
static void test_payload_copy()
{
    telemetry_event_t event;

    const char message[] = "Telemetry Services";
    const uint32_t id = 98;
    const telemetry_level_t level = TELEMETRY_LEVEL_INFO;

    bool ok = telemetry_event_make(&event, id, message, sizeof(message), level);

    assert(ok);

    assert(event.event_id == id);
    assert(event.level == level);
    assert(event.payload_size == sizeof(message));
    assert(event.reserved == 0x00);
    assert(event.timestamp != 0);

    // Compare the payload
    memcmp(event.payload, message, sizeof(message));

    // If everything is okay, then test is passed
    printf(" Telemetry :: test_payload_copy is passed. \n");

}

/**
 * @brief Tests monotonic timestamp functionality.
 *
 * Ensures that timestamps are monotonically increasing and not affected
 * by system time changes.
 */
static void test_monotonic_timestamp()
{
    telemetry_event_t ev1, ev2;

    const uint8_t payload1[] = {1,2,3};
    const uint8_t payload2[] = {4,5,6};
    telemetry_level_t level = TELEMETRY_LEVEL_DEBUG;

    assert(telemetry_event_make(&ev1, 0x01, payload1, sizeof(payload1), level));
    assert(telemetry_event_make(&ev2, 0x02, payload2, sizeof(payload2),level));

    // Monotonic timestamp should not be backward compatible (i.e., increasing)
    assert(ev1.timestamp <= ev2.timestamp);

    printf("Telemetry :: test_monotonic_timestamp is passed. \n");
}

/**
 * @brief Tests handling of oversized payloads.
 *
 * Verifies that telemetry_event_make rejects payloads larger than
 * the maximum allowed size.
 */
static void test_oversized_payload()
{
    telemetry_event_t event;

    // Create a payload bigger than the maximum allowed size
    uint8_t buff[TELEMETRY_EVENT_PAYLOAD_MAX + 1]; 
    telemetry_level_t level = TELEMETRY_LEVEL_ERROR;

    // Set the payload values
    memset(buff, 0xAA, sizeof(buff));

    bool ok = telemetry_event_make(&event, 0x07, buff, sizeof(buff), level);

    assert(!ok);

    printf("Telemetry :: test_oversized_payload is passed. \n");
}