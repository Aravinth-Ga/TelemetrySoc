/**
 * @file telemetry_agent.h
 * @brief Telemetry agent header.
 *
 * Defines the interface for the telemetry agent that handles sending events.
 *
 * @author Aravinthraj Ganesan
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
    extern "C" {
#endif


// Forward declarations to avoid including heavy headers
typedef struct oasl_wakeup osal_wakeup_t;
typedef struct osal_thread osal_thrad_t;

// Structure for transport interface (can wrap C++ or C objects)
typedef struct transport_c {
    void* ctx;  // Pointer to transport context (could be a C++ object)
    bool (*send_event)(void* ctx, const telemetry_event_t* ev);  // Function to send an event
    void (*shutdown)(void* ctx); // Optional shutdown function
} transport_c_t;

/*
 * The telemetry agent connects:
 * 1. Producer thread -> ring buffer (puts events here)
 * 2. Producer thread -> wakeup notification (wakes the agent)
 * 3. Agent thread -> drains ring buffer -> sends events via transport
 */
typedef struct telemetry_agent telemetry_agent_t;

/**
 * @brief Starts the telemetry agent.
 *
 * Creates a background thread that waits for events in the ring buffer and sends them using the transport.
 *
 * @param out_agent Pointer to store the created agent.
 * @param ring_handle The ring buffer to read events from.
 * @param transport The transport to send events with.
 * @return true if started successfully, false otherwise.
 */
bool telemetry_agent_start(telemetry_agent_t** out_agent, ring_buffer_t* ring_handle, transport_c_t* transport);

/**
 * @brief Stops the telemetry agent.
 *
 * Tells the agent to stop processing and cleans up.
 *
 * @param agent The agent to stop.
 */
void telemetry_agent_stop(telemetry_agent_t* agent);

/**
 * @brief Notifies the telemetry agent.
 *
 * Wakes up the agent's thread to check for new events.
 *
 * @param agent The agent to notify.
 */
void telemetry_agent_notify(telemetry_agent_t* agent);

/**
 * @brief Gets the number of events sent.
 *
 * Returns how many events have been successfully sent so far.
 *
 * @param agent The agent to query.
 * @return Number of sent events.
 */
uint64_t telemetry_agent_sent_count(const telemetry_agent_t* agent);

/**
 * @brief Gets the number of wakeups.
 *
 * Returns how many times the agent has been woken up.
 *
 * @param agent The agent to query.
 * @return Number of wakeups.
 */
uint64_t telemetry_agent_wakeup_count(const telemetry_agent_t* agent);



#ifdef __cplusplus
    }
#endif