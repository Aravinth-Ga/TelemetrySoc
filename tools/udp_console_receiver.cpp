/**
 * @file udp_console_receiver.cpp
 * @brief UDP console receiver tool.
 *
 * Receives UDP datagrams on a fixed port and prints them to the console.
 *
 * @author Aravinthraj Ganesan
 */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdio>
#include <cstring>
#include <string>
#include <cstdlib>
namespace {


constexpr uint16_t kListenPort = 9000;
constexpr size_t kMaxDatagrambytes = 2048;

/**
 * @brief Creates and binds a UDP socket for receiving datagrams.
 *
 * Creates a UDP socket and binds it to listen on all network interfaces
 * (0.0.0.0) at the specified port.
 *
 * @param port The port number to listen on.
 * @return Socket file descriptor on success, or -1 on failure.
 */
int create_and_bind_udp_receiver_socket(uint16_t port)
{

    // Create UDP socket
    const int socket_fd = ::socket(AF_INET, SOCK_DGRAM, 0);

    if(socket_fd < 0)
    {
        std::perror("socket");
        return -1;
    }

    // Enable port reuse to avoid "address already in use" errors
    int reuse = 1;
    (void)::setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    // Prepare the address structure to listen on all interfaces
    sockaddr_in local_address{};
    local_address.sin_family = AF_INET;
    local_address.sin_port = htons(port);
    local_address.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind the socket to the address and port
    if(::bind(socket_fd, reinterpret_cast<sockaddr*>(&local_address), sizeof(local_address)) < 0)
    {
        std::perror("bind");
        ::close(socket_fd);
        return -1;
    }

    return socket_fd;

}

/**
 * @brief Converts a socket address to a readable "ip:port" format.
 *
 * Converts an IPv4 address structure to a human-readable string
 * in the format "ip:port".
 *
 * @param sender       The sender socket address structure.
 * @param out          Buffer to store the formatted address string.
 * @param out_capacity Size of the output buffer.
 */
void format_sender(const sockaddr_in& sender, char* out, size_t out_capacity)
{
    char ip[INET_ADDRSTRLEN];

    // Clear the IP buffer
    ::memset(ip, 0, sizeof(ip));

    // Convert binary IP address to string format
    ::inet_ntop(AF_INET, &sender.sin_addr, ip, sizeof(ip));

    // Extract the port number
    const uint16_t port = ntohs(sender.sin_port);

    // Format the address as "ip:port"
    ::snprintf(out, out_capacity, "%s:%u", ip, port);

}

}


/**
 * @brief Program entry point.
 *
 * @param arg_count  Number of command-line arguments.
 * @param arg_vector Array of strings; each string is one argument.
 * @return 0 on normal exit.
 */
int main(int arg_count, char** arg_vector)
{
    uint16_t port = kListenPort;

    // Allow optional port override via command-line argument
    if(arg_count == 2)
    {
        const long port_loc = std::strtol(arg_vector[1], nullptr, 10);
        if(port_loc > 0 && port_loc < 65535)
        {
            port = static_cast<uint16_t>(port_loc);
        }
    }

    // Create and bind the UDP socket
    const int receiver_fd = create_and_bind_udp_receiver_socket(port);

    // Check if socket creation was successful
    if(receiver_fd < 0)
    {
        std::fprintf(stderr, "Failed to start the UDP receiver on port %u\n", port);
        return 1;
    }

    std::printf("UDP console receiver started listening on 0.0.0.0:%u\n", port);
    std::printf("Press Ctrl+C to stop.\n\n");

    // Continuously receive and display incoming UDP datagrams
    while(1)
    {
        char datagram_msg[kMaxDatagrambytes];
        
        sockaddr_in sender{};
        socklen_t sender_len = sizeof(sender);

        // Receive UDP datagram from the socket
        const ssize_t bytes = ::recvfrom(receiver_fd, datagram_msg, sizeof(datagram_msg) - 1, 0, 
                                         reinterpret_cast<sockaddr*>(&sender), &sender_len);

        if(bytes < 0)
        {
            std::perror("recvfrom");
            continue;
        }

        // Null-terminate the received data to make it printable
        datagram_msg[bytes] = '\0';

        // Convert sender address to readable format
        char sender_text[64];
        format_sender(sender, sender_text, sizeof(sender_text));

        // Display the received message with sender information
        std::printf(" From %-21s | %zd bytes | %s", sender_text, bytes, datagram_msg);

        // Add newline if the message did not include one
        if(bytes > 0 && datagram_msg[bytes - 1] != '\n')
        {
            std::printf("\n");
        }
    }

    // Close the UDP socket
    ::close(receiver_fd);

    return 0;
}

