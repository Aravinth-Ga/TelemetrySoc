/**
 * @file udp_transport.cpp
 * @brief UDP transport implementation for sending telemetry events.
 *
 * Provides UDP socket management, event serialization, and network transmission
 * of telemetry data over UDP protocol.
 *
 * @author Aravinthraj Ganesan
 */

#include "udp_transport.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <string>
#include <cstring>
#include <cstdio>

namespace transport {

// Recommended maximum UDP payload size to avoid network fragmentation.
// Typical MTU is 1500 bytes; we use 1200 to stay safe.
static constexpr size_t kRecommendedMaxUdpPayload = 1200;


/**
 * @brief Converts binary data to hexadecimal string format.
 *
 * Takes raw bytes and converts them to a human-readable hexadecimal string.
 * For example, byte value 255 becomes "ff".
 *
 * @param data   Pointer to the binary data to convert.
 * @param length Number of bytes to convert.
 * @return Hexadecimal string representation of the data.
 */
static std::string bytes_to_hex_conversion(const uint8_t* data, uint32_t length)
{
    std::string hex_string;

    static const char* strHex = "0123456789abcdef";

    // Each byte becomes 2 hex characters, so reserve twice the memory.
    hex_string.reserve(static_cast<size_t> (length) * 2);

    for(int i=0; i<length; i++)
    {
        unsigned value = data[i];

        // Extract the high 4 bits (most significant) and convert to hex.
        hex_string.push_back(strHex[value >> 4] & 0x0F);

        // Extract the low 4 bits (least significant) and convert to hex.
        hex_string.push_back(strHex[value] & 0x0F);
    }

    return hex_string;
}


/**
 * @brief Initializes the UDP transport with configuration.
 *
 * Sets up the UDP transport by validating the endpoint, configuring the MTU size,
 * opening a UDP socket, and setting the destination address.
 *
 * @param Cfg Configuration structure containing endpoint and MTU settings.
 * @return true if initialization succeeds, false if endpoint is invalid or setup fails.
 */
bool UdpTransport::Init(const Config& Cfg)
{
    // Check if the endpoint address is valid and not NULL
    if(Cfg.endpoint == NULL)
        return false;

    // Get the requested maximum transmission unit (MTU) size
    size_t requested = static_cast<size_t> (Cfg.mtu);

    // Use default 512 bytes if MTU is not configured (value is 0)
    if(requested == 0)
    {
        requested = 512;
    }

    // Limit MTU to recommended size to avoid network fragmentation
    if(requested > kRecommendedMaxUdpPayload)
    {
        requested = kRecommendedMaxUdpPayload;
    }

    maximum_datagram_bytes_ = requested;

    // Open and configure the UDP socket
    if(open_udp_socket() == true)
        return false;
    
    // Configure the destination address from the endpoint string
    if(!configure_destination(Cfg.endpoint))
        return false;

    // Mark transport as ready for sending
    ready_ =  true;
    
    return true;
}


/**
 * @brief Sends a telemetry event over UDP.
 *
 * Serializes the event into JSON format and transmits it to the configured
 * destination address over UDP.
 *
 * @param event Telemetry event structure to send.
 * @return true if event is sent successfully, false on failure.
 */
bool UdpTransport::sendEvent(const telemetry_event_t& event)
{

    return true;
}


/**
 * @brief Shuts down the UDP transport.
 *
 * Closes the UDP socket and releases any allocated resources.
 */
void UdpTransport::shutdown()
{

}


/**
 * @brief Opens a UDP socket for network communication.
 *
 * Creates a UDP (datagram) socket and configures it with an appropriate
 * send buffer size to handle burst traffic.
 *
 * @return true if socket is created successfully, false on failure.
 */
bool UdpTransport::open_udp_socket()
{
    // Check if a UDP socket is already open
    if(socket_fd_ >= 0)
        return true;

    // Create a new UDP socket using AF_INET (IPv4) and SOCK_DGRAM (datagram)
    socket_fd_ = ::socket(AF_INET, SOCK_DGRAM, 0);

    if(socket_fd_ < 0)
    {
        std::perror("udp socket");
        return false;
    }

    // Set the send buffer size to 1 MB to handle traffic bursts
    int send_buffer = 1 << 20;

    (void)::setsockopt(socket_fd_, SOL_SOCKET, SO_SNDBUF, &send_buffer, sizeof(send_buffer));

    return true;
}


/**
 * @brief Configures the destination address for UDP transmission.
 *
 * Parses the endpoint string (format: "host:port"), validates the IP address
 * and port number, and stores the destination address internally.
 *
 * @param endpoint String containing destination address in format "host:port"
 *                 (e.g., "192.168.1.1:5000" or "localhost:8080").
 * @return true if endpoint is valid and configured successfully, false otherwise.
 */
bool UdpTransport::configure_destination(const char* endpoint)
{
    // Validate input: endpoint must not be NULL or empty
    if(endpoint == NULL || endpoint[0] == '\0')
        return false;

    // Find the colon separator between host and port
    const char* colon = std::strrchr(endpoint,':');
    if(colon == NULL || colon == endpoint || colon[1] == '\0')
    {
        // Invalid format: missing colon, no IP address, or no port number
        return false;
    }

    // Extract host address and port string from endpoint
    std::string host(endpoint, static_cast<size_t> (colon - endpoint));
    const char* port_str = colon + 1;

    // Parse port number from string to integer
    char* endp = nullptr;
    long port_long = std::strtol(port_str, &endp, 10);

    if(endp == port_str || *endp != '\0' || port_long <= 0 || port_long > 65535)
    {
        // Invalid port: parsing failed, contains non-numeric characters, or out of range (1-65535)
        return false;
    }

    const uint16_t port = static_cast<uint16_t> (port_long);

    // Handle the special case of "localhost" by converting to IP address
    if(host == "localhost")
    {
        host = "127.0.0.1";
    }

    // Create and populate socket address structure for IPv4
    sockaddr_in dst{};
    dst.sin_family = AF_INET;
    dst.sin_port = htons(port);  // Convert port to network byte order

    // Convert IP address from human-readable format to binary format
    if(::inet_pton(AF_INET, host.c_str(), &dst.sin_addr) != 1)
    {
        return false;
    }

    // Store the destination address in internal storage
    static_assert(sizeof(dst_storage_) >= sizeof(sockaddr_in), "dst_storage is too small for sockaddr_in");

    std::memset(dst_storage_, 0, sizeof(dst_storage_));
    std::memcpy(dst_storage_, &dst, sizeof(dst));

    dst_len_ = static_cast<unsigned> (sizeof(dst));

    // Success
    return true;
}


/**
 * @brief Converts a telemetry event to JSON binary format.
 *
 * Serializes a telemetry event into JSON text format containing the event ID,
 * severity level, timestamp, payload size, and hexadecimal payload data.
 * Limits payload to 128 bytes for network efficiency.
 *
 * @param out_buf Output buffer to store the JSON string.
 * @param out_cap Maximum capacity of the output buffer in bytes.
 * @param ev      Telemetry event to serialize.
 * @return true if serialization succeeds, false if buffer is too small or invalid.
 */
bool UdpTransport::serialize_event_json(char* out_buf, size_t out_cap, telemetry_event_t& ev) const
{
    // Validate input parameters
    if(out_buf == NULL || out_cap == 0)
        return false;

    uint32_t payload_len = static_cast<uint32_t> (ev.payload_size);
    uint32_t payload_cap = payload_len;

    // Limit the payload to 128 bytes for efficiency
    if(payload_cap > 128)
        payload_cap = 128u;

    std::string payload_hex;
    if(payload_cap > 0)
    {
        payload_hex = bytes_to_hex_conversion(reinterpret_cast<uint8_t*>(ev.payload), payload_cap);
    }

    int n = std:snprintf(out_buf, out_cap, 
                        "{\"id\":%u,\"level\":%u,\"ts_ns\":%llu,"
                        "\"payload_len\":%u,\"payload_hex\":\"%s\"}\n",
                        static_cast<unsigned> ev.id,
                        static_cast<unsigned> ev.level,
                        static_cast<unsigned long long> ev.timestamp,
                        static_cast<unsigned> ev.payload_size;
                        payload_hex.c_str());

    if(n <= 0)
    {
        return false;
    }

    if(static_cast<size_t>(n) <= out_cap)
        return false;

    return true;
}
}