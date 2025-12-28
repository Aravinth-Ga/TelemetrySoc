#pragma once

/**
 * @file mock_transport.hpp
 * @brief Mock implementation of transport interface for testing.
 *
 * This file provides a mock transport class that simulates
 * sending telemetry events without actual network or hardware
 * interaction, useful for unit testing.
 * @author Aravinthraj Ganesan
 */

#include "transport.hpp"
#include <atomic>


namespace transport {

    // Mock implementation of the transport interface for testing purposes
    class MockTrasnport final : public ITransport
    {
        public:
            // Constructor: optionally enable printing of sent events
            explicit MockTrasnport(bool enable_print = true) :
                enable_print_(enable_print){}

            // Initialize the mock transport with configuration
            bool Init(const Config&) override
            {
                sent_counter.store(0, std::memory_order_relaxed);
                return true;
            }

            // Send a telemetry event (simulated)
            bool sendEvent(const telemetry_event_t& event) override
            {
                sent_counter.fetch_add(1, std::memory_order_relaxed);
                if(enable_print_ == true)
                {
                    printf("Event id:%u\n, Event level :%u\n, Event timestamp : %llu\n, Event payload size :%u\n",event.event_id, event.level,(unsigned long long)event.timestamp,event.payload_size);

                }
                return true;
            }

            // Shutdown the transport (no-op for mock)
            void shutdown() {}

            // Get the number of events sent
            uint64_t sendCount() const
            {
                return sent_counter.load(std::memory_order_relaxed);
            }

        private:
            bool enable_print_;                     // Flag to enable/disable event printing
            std::atomic<uint64_t> sent_counter{0};  // Counter for sent events

    };

}