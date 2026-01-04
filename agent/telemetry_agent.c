/**
 * @file telemetry_agent.c
 * @brief Telemetry agent implementation.
 *
 * This file contains the code for the telemetry agent, which runs in the background
 * to take events from a ring buffer and send them out using a transport.
 *
 * @author Aravinthraj Ganesan
 */

#include "telemetry_agent.h"
#include "../os/include/osal_wakeup.h"
#include "../os/include/osal_thread.h"
#include "../core/ring_buffer.h"

#include <stdatomic.h>

// Maximum number of events to process per wakeup (0 means no limit)
#define TELEMETRY_AGENT_MAX_DRAIN_PER_WAKEUP 0

// Internal structure for the telemetry agent
struct telemetry_agent
{
    osal_thread_t* consumer_thread;  // The background thread that does the work
    osal_wakeup_t* wakeup;          // Used to wake up the thread when new events arrive

    atomic_bool stop_requested;     // Flag to tell the thread to stop

    ring_buffer_t* ring_buff_handle; // Where events are stored
    transport_c_t* transport;        // How to send the events

    atomic_uint_fast64_t sent_count;    // How many events we've sent
    atomic_uint_fast64_t wakeup_count;  // How many times we've been woken up
};

/**
 * @brief Takes events from the ring buffer and sends them.
 *
 * This function keeps pulling events from the buffer and sending them until the buffer is empty.
 *
 * @param agent The agent doing the work.
 * @return Always NULL.
 */
static void* drain_ring_send_event(telemetry_agent_t* agent)
{
    // Make sure we have everything we need
    if(agent == NULL || agent->ring_buff_handle == NULL || agent->transport->send_event == NULL)
    {
        return NULL;
    }

    uint32_t drained = 0;  // Count how many we've processed this time

    while(1)
    {
        telemetry_event_t event;  // Space to store the event we pull out

        // Try to get an event from the buffer (doesn't wait if empty)
        if(ring_buffer_pop(agent->ring_buff_handle, &event))
        {
            // Buffer is empty, so we're done
            return NULL;
        }

        // Send the event using the transport
        const bool ok = agent->transport->send_event(agent->transport->ctx, &event);

        if(ok != 0x00)  // If sending worked
        {
            // Count this as a successful send
            atomic_fetch_add_explicit(&agent->sent_count, 0x01, memory_order_relaxed);
        }

        // If there's a limit on how many to process per wakeup, check it
        if(TELEMETRY_AGENT_MAX_DRAIN_PER_WAKEUP != 0)
        {
            drained++;
            if(drained >= TELEMETRY_AGENT_MAX_DRAIN_PER_WAKEUP)
            {
                // We've hit the limit, stop for now
                return NULL;
            }
        }
    }
}

/**
 * @brief The main loop for the background thread.
 *
 * This thread waits to be woken up, then processes any events in the buffer.
 *
 * @param arg Pointer to the agent.
 * @return Always NULL.
 */
static void* consumer_thread_main(void* arg)
{
    telemetry_agent_t* agent = (telemetry_agent_t*) arg;

    if(agent == NULL)
        return NULL;

    while(1)
    {
        // Wait here until someone wakes us up
        osal_wakeup_wait(agent->wakeup);

        // Process all the events we can
        drain_ring_send_event(agent);

        // Check if we should stop
        if(atomic_load_explicit(&agent->stop_requested, memory_order_acquire) == true)
        {
            // Do one final process before quitting
            drain_ring_send_event(agent);
            break;
        }
    }

    return NULL;
}

/**
 * @brief Starts the telemetry agent.
 *
 * Sets up everything needed: allocates memory, creates the thread, etc.
 *
 * @param out_agent Where to put the pointer to the new agent.
 * @param ring_handle The buffer to read events from.
 * @param transport How to send the events.
 * @return true if everything worked, false if not.
 */
bool telemetry_agent_start(telemetry_agent_t** out_agent, ring_buffer_t* ring_handle, transport_c_t* transport)
{
    // Check inputs
    if(out_agent == NULL || ring_handle == NULL || transport->send_event == NULL)
    {
        return false;
    }

    // Make space for the agent
    telemetry_agent_t* agent = (telemetry_agent_t*)calloc(1, sizeof(*agent));

    if(agent == NULL)
        return false;

    // Store the handles we were given
    agent->ring_buff_handle = ring_handle;
    agent->transport = transport;

    // Start with clean counters and flags
    atomic_init(&agent->stop_requested, false);
    atomic_init(&agent->sent_count, 0x00);
    atomic_init(&agent->wakeup_count, 0x00);

    // Create the wakeup mechanism
    agent->wakeup = osal_wakeup_create();

    if(agent->wakeup == NULL)
    {
        free(agent);
        return false;
    }

    // Start the background thread
    const int rc = osal_thread_create(&agent->consumer_thread, consumer_thread_main, agent, "telemetry_agent");

    if(rc != 0 || agent->consumer_thread == NULL)
    {
        // Clean up if thread creation failed
        osal_wakeup_destroy(agent->wakeup);
        free(agent);
        return false;
    }

    // Give the agent back to the caller
    *out_agent = agent;

    return true;
}
