#pragma once

/**
 * @file transport.hpp
 * @brief Interface for transport mechanisms in telemetry system.
 *
 * This file defines the ITransport interface and Config struct
 * for handling telemetry event transmission across different
 * transport protocols.
 * @author Aravinthraj Ganesan
 */

extern "C" {
    #include "../core/event.h"
}

namespace transport {

    // Configuration structure for transport settings
    struct Config
    {
        // declare the memeber for port access and chunksize
        const char* endpoint = NULL;        // later have to defined the port address here
        uint32_t mtu = 512;                 // Maximum transmission unit/ chunksize
    };

    // Interface for Transport
    class ITransport{
    public:
        virtual ~ITransport() = default;                            // Virtual destructor for proper cleanup
        virtual bool Init(const Config &cfg) = 0;                   // Initialize the transport with config
        virtual bool sendEvent(const telemetry_event_t &event) = 0; // Send a telemetry event
        virtual void shutdown() = 0;                                // Shutdown the transport

    };
}
