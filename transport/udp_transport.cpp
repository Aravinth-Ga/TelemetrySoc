#include "udp_transport.hpp"

// Implementation of UDP transport for sending telemetry events.

namespace transport {


    // avoid fragmentation in common MTUs
    static constexpr size_t kRecommendedMaxUdpPayload = 1200;

    // Set up the UDP transport with the given configuration
    // Checks endpoint, sets data size, opens socket, and configures destination
    // Returns true if successful, false otherwise
    bool UdpTransport::Init(const Config& Cfg)
    {
        // Check if the endpoint is valid
        if(Cfg.endpoint == NULL)
            return false;

        // Set the maximum data size (MTU)
        size_t requested = static_cast<size_t> (Cfg.mtu);

        // Use default size 512 if MTU is not configured
        if(requested == 0)
        {
            requested = 512;
        }

        // Limit to maximum UDP size to avoid fragmentation
        if(requested > kRecommendedMaxUdpPayload)
        {
            requested = kRecommendedMaxUdpPayload;
        }

        maximum_datagram_bytes_ = requested;

        // Open the UDP socket
        if(!open_udp_socket)
            return false;
        
        // Set up the destination endpoint
        if(!configure_destination(Cfg.endpoint))
            return false;

        // Mark transport as ready
        ready_ =  true;
        
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