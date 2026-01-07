#include "udp_transport.hpp"

// Implementation of UDP transport for sending telemetry events.

namespace transport {

    // Initializes the UDP transport with config
    bool UdpTransport::Init(const Config& Cfg)
    {
        if(Cfg == NULL)
            return false;

        
        return true;
    }

    // Sends a telemetry event over UDP
    bool UdpTransport::sendEvent(const telemetry_event_t& event)
    {

        return true;
    }

    // Shuts down the UDP transport
    void UdpTransport::shutdown()
    {

    }

    // Opens a UDP socket
    bool UdpTransport::open_udp_socket()
    {

        return true;
    }

    // Sets up the destination address
    bool UdpTransport::configure_destination(const char* endpoint)
    {
        return true;
    }

    // Converts event to JSON format
    bool UdpTransport::serialize_event_json(char* out_buf, size_t out_cap, telemetry_event_t& event) const
    {
        return true;
    }
}