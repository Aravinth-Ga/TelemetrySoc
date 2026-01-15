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
#define TELEMETRY_AGENT_MAX_DRAIN_PER_WAKEUP 50

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
 * Pulls events from the buffer and sends them until empty.
 *
 * @param agent The agent doing the work.
 * @return Always NULL.
 */
static void* drain_ring_send_event(telemetry_agent_t* agent)
{
    // Check inputs
    if(agent == NULL || agent->ring_buff_handle == NULL || agent->transport->send_event == NULL)
    {
        return NULL;
    }

    uint32_t drained = 0;  // Count processed events

    while(1)
    {
        telemetry_event_t event;  // Space for event

        // Get event from buffer (no wait if empty)
        if(!ring_buffer_pop(agent->ring_buff_handle, &event))
        {
            // Buffer empty, done
            return NULL;
        }

        // Send event
        const bool ok = agent->transport->send_event(agent->transport->context, &event);

        if(ok)  // If send succeeded
        {
            // Increment sent count
            atomic_fetch_add_explicit(&agent->sent_count, 1, memory_order_relaxed);
        }

        // Check drain limit
        if(TELEMETRY_AGENT_MAX_DRAIN_PER_WAKEUP != 0)
        {
            drained++;
            if(drained >= TELEMETRY_AGENT_MAX_DRAIN_PER_WAKEUP)
            {
                // Hit limit, stop
                return NULL;
            }
        }
    }
}

/**
 * @brief The main loop for the background thread.
 *
 * Waits for wakeup, then processes events.
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
        // Wait for wakeup
        osal_wakeup_wait(agent->wakeup);

        // Process events
        drain_ring_send_event(agent);

        // Check stop flag
        if(atomic_load_explicit(&agent->stop_requested, memory_order_acquire) == true)
        {
            // Final process before exit
            drain_ring_send_event(agent);
            break;
        }
    }

    return NULL;
}

/**
 * @brief Starts the telemetry agent.
 *
 * Creates agent, thread, and wakeup mechanism.
 *
 * @param out_agent Where to store the agent pointer.
 * @param ring_handle The buffer to read from.
 * @param transport How to send events.
 * @return true on success, false on failure.
 */
bool telemetry_agent_start(telemetry_agent_t** out_agent, ring_buffer_t* ring_handle, transport_c_t* transport)
{
    // Check inputs
    if(out_agent == NULL || ring_handle == NULL || transport->send_event == NULL)
    {
        return false;
    }

    // Allocate agent
    telemetry_agent_t* agent = (telemetry_agent_t*)calloc(1, sizeof(*agent));

    if(agent == NULL)
        return false;

    // Set handles
    agent->ring_buff_handle = ring_handle;
    agent->transport = transport;

    // Init atomics
    atomic_init(&agent->stop_requested, false);
    atomic_init(&agent->sent_count, 0);
    atomic_init(&agent->wakeup_count, 0);

    // Create wakeup
    agent->wakeup = osal_wakeup_create();

    if(agent->wakeup == NULL)
    {
        free(agent);
        return false;
    }

    // Create thread
    const int rc = osal_thread_create(&agent->consumer_thread, consumer_thread_main, agent, "telemetry_agent");

    if(rc != 0 || agent->consumer_thread == NULL)
    {
        // Cleanup on failure
        osal_wakeup_destroy(agent->wakeup);
        free(agent);
        return false;
    }

    // Return agent
    *out_agent = agent;

    return true;
}
/**
 * @brief Notifies the telemetry agent.
 *
 * Wakes up the agent to process events.
 *
 * @param agent The agent to notify.
 */
void telemetry_agent_notify(telemetry_agent_t* agent)
{
    // Check input
    if(agent == NULL)
    {
        return;
    }

    // Increment wakeup count
    atomic_fetch_add_explicit(&agent->wakeup_count, 1, memory_order_relaxed);

    // Wake the thread
    osal_wakeup_notify(agent->wakeup);
}

/**
 * @brief Stops the telemetry agent.
 *
 * Signals stop, joins thread, and cleans up.
 *
 * @param agent The agent to stop.
 */
void telemetry_agent_stop(telemetry_agent_t* agent)
{
    // Check input
    if(agent == NULL)
        return;

    // Set stop flag
    atomic_store_explicit(&agent->stop_requested, true, memory_order_relaxed);

    // Wake thread to exit
    osal_wakeup_notify(agent->wakeup);

    // Join thread
    osal_thread_join(agent->consumer_thread);
    // Destroy thread
    osal_thread_destroy(agent->consumer_thread);
    agent->consumer_thread = NULL;

    // Destroy wakeup
    osal_wakeup_destroy(agent->wakeup);
    agent->wakeup = NULL;

    // Shutdown transport
    if(agent->transport->shutdown)
    {
        agent->transport->shutdown(agent->transport->context);
    }

    free(agent);
}

/**
 * @brief Gets the sent event count.
 *
 * @param agent The agent.
 * @return Number of events sent.
 */
uint64_t telemetry_agent_sent_count(const telemetry_agent_t* agent)
{
    if(agent == NULL)
        return 0;
    
    return atomic_load_explicit(&agent->sent_count, memory_order_relaxed);
}

/**
 * @brief Gets the wakeup count.
 *
 * @param agent The agent.
 * @return Number of wakeups.
 */
uint64_t telemetry_agent_wakeup_count(const telemetry_agent_t* agent)
{
    if(agent == NULL)
        return 0;

    return atomic_load_explicit(&agent->wakeup_count, memory_order_relaxed);
}
