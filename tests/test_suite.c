/**
 * @file test_suite.c
 * @brief Main test suite for telemetry functionality.
 *
 * This file contains the main entry point for running all unit tests
 * for the telemetry system, including event and ring buffer tests.
 * @author Aravinthraj Ganesan
 */

#include "test_suite.h"

void main()
{
    // Test the event function
    test_event();
    // Test the ring buffer functionality
    test_ring_buffer();
}