#pragma once

#include "transport.hpp"

// This module provides UDP transport for sending telemetry events.
// It implements the ITransport interface to send data over UDP sockets.

namespace transport {

    class UdpTransport final : public ITransport
    {
        public:
            // Default constructor
            UdpTransport() = default;
            // Destructor
            ~UdpTransport() override;

            // Initializes the UDP transport with the given configuration
            bool Init(const Config& cfg) override;
            // Sends a telemetry event over UDP
            bool sendEvent(const telemetry_event_t& event) override;
            // Shuts down the UDP transport and cleans up resources
            void shutdown() override;

        private:
            // Opens a UDP socket for communication
            bool open_udp_socket();
            // Sets up the destination address for sending data
            bool configure_destination(const char* endpoint);
            // Converts the telemetry event to JSON format for transmission
            bool serialize_event_json(char* out_buf, size_t out_cap, const telemetry_event_t& event) const;

        private:
            // File descriptor for the UDP socket
            int socket_fd_ = -1;
            // Flag to check if the transport is ready to send data
            bool ready_ = false;

            // Maximum size of a UDP datagram in bytes
            size_t maximum_datagram_bytes_ = 512;

            // Storage for the destination address, aligned to 8 bytes for safe casting
            // This ensures the address starts at a memory location divisible by 8,
            // preventing undefined behavior during type conversions
            alignas(8) unsigned char dst_storage_[32];

            unsigned dst_len_ = 0;
    };

}