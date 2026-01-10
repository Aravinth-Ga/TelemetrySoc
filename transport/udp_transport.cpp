#include "udp_transport.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <string>
#include <cstring>
#include <cstdio>

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
        if(open_udp_socket() == true)
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
    // Creates and configures a UDP socket for sending data
    // Returns true if successful, false otherwise
    bool UdpTransport::open_udp_socket()
    {
        // Check if the UDP socket is already open
        if(socket_fd_ >= 0)
            return true;

        // Create a new UDP socket
        socket_fd_ = ::socket(AF_INET, SOCK_DGRAM, 0);

        if(socket_fd_ < 0)
        {
            std::perror("udp socket");
            return false;
        }

        // Set the send buffer size to 1MB for the burst tolerance
        int send_buffer = 1 << 20;

        (void)::setsockopt(socket_fd_, SOL_SOCKET, SO_SNDBUF, &send_buffer, sizeof(send_buffer));

        return true;
    }

    // Sets up the destination address for UDP transmission.
    // Parses the endpoint string (format: "host:port"), validates it, and stores the address.
    // Parameters: endpoint - string in format "host:port" (e.g., "192.168.1.1:5000").
    // Returns: true if successful, false if endpoint is invalid or cannot be parsed.
    bool UdpTransport::configure_destination(const char* endpoint)
    {
        // 1. Validate input: endpoint must not be NULL or empty.
        if(endpoint == NULL || endpoint[0] == '\0')
            return false;

        // 2. Find the colon separator between host and port.
        const char* colon = std::strrchr(endpoint,':');
        if(colon == NULL || colon == endpoint || colon[1] == '\0')
        {
            // Invalid format: missing colon, missing IP address, or missing port number.
            return false;
        }

        // 3. Extract host address and port string.
        std::string host(endpoint, static_cast<size_t> (colon - endpoint));
        const char* port_str = colon + 1;

        // 4. Parse port number from string to integer.
        char* endp = nullptr;
        long port_long = std::strtol(port_str, &endp, 10);

        if(endp == port_str || *endp != '\0' || port_long <= 0 || port_long > 65535)
        {
            // Invalid port: parsing failed, non-numeric characters present, or out of valid range (1-65535)
            return false;
        }

        const uint16_t port = static_cast<uint16_t> (port_long);

        // 5. Handle localhost special case.
        if(host == "localhost")
        {
            host = "127.0.0.1";
        }

        // 6. Populate socket address structure.
        sockaddr_in dst{};
        dst.sin_family = AF_INET;
        dst.sin_port = htons(port);  // Convert port to network byte order (big-endian)

        // 7. Convert IP address from human-readable format to binary format.
        if(::inet_pton(AF_INET, host.c_str(), &dst.sin_addr) != 1)
        {
            return false;
        }

        // 8. Store the destination address in internal storage.
        static_assert(sizeof(dst_storage_) >= sizeof(sockaddr_in), "dst_storage is too small for sockaddr_in");

        std::memset(dst_storage_, 0, sizeof(dst_storage_));
        std::memcpy(dst_storage_, &dst, sizeof(dst));

        dst_len_ = static_cast<unsigned> (sizeof(dst));

        // 9. Return true on success.
        return true;
    }

    // Converts event to JSON format
    bool UdpTransport::serialize_event_json(char* out_buf, size_t out_cap, telemetry_event_t& event) const
    {
        return true;
    }
}