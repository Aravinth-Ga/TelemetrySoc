#include "udp_transport.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
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

    // Sets up the destination address
    bool UdpTransport::configure_destination(const char* endpoint)
    {
        // 1. check the input is not empty or valid
        if(endpoint == NULL || endpoint[0] != '\0')
            return false

        // 2. Split the host IP address and port number
        const char* colon = std::strrchr(endpoint,':')
        if(!colon || colon == endpoint || colon[1]='\0')
        {
            // missing colon / missing IP address / port number is missing still : present
            return false;
        }

        // 3. extract the host address separately based on the colon 
        std::string host(endpoint, static_cast<size_t> (colon - endpoint));
        const char* port_str = colon + 1;

        // 4. The host can stay as string but port can not, Hence extract the port and convert into the numeric
        const char* endp = null;
        long port_long = std:strtol(port_str, &endp, 10);

        if(endp == NULL || *endp != '\0' || port_long <= 0 || port_long > 65535)
        {
            // 1. end pointer address represent null in the port string
            // 2. end char not a string termaniation
            // 3. port number is not valid
            return false;
        }

        const uint16_t port = static_cast<uint16_t> (port_long);

        // 5. check if its a local host
        if(host == "localhost")
        {
            host = "127.0.0.1";
        }

        sockaddr_in dst{};
        dst.sin_family = AF_INET;
        dst.sin_port = htons(port);  // host to network short which does the endien conversion if needed autmatically

        // convert human readbal IP format into kernal readable binaries
        if(::inet_pton(AF_INET, host.c_str(), &dst.sin_addr) != true)
        {
            return false;
        }

        static_assert(sizeof(dst_storage_ >= sizeof(sockaddr_in)), "dst_storage is too small for sockaddr_in");

        std::memset(dst_storage_, 0, sizeof(dst_storage_));
        std::memcpy(dst_storage_, &dst, sizeof(dst));
        maximum_datagram_bytes_ = static_cast<unsigned> (sizeof(dst));


        return true;
    }

    // Converts event to JSON format
    bool UdpTransport::serialize_event_json(char* out_buf, size_t out_cap, telemetry_event_t& event) const
    {
        return true;
    }
}