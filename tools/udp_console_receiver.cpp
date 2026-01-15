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
#include <string>
#include <cstdlib>
namespace {


constexpr uint16_t kListenPort = 9000;
constexpr size_t kMaxDatagrambytes = 2048;

// create a udp socket and bind it to "0.0.0.0:<port>"

int create_and_bind_udp_receiver_socket(uint16_t port)
{

    // Create a UDP socket
    const int socket_fd = ::socket(AF_INET, SOCK_DGRAM, 0);

    if(socket_fd < 0)
    {
        std::perror("socket");
        return -1;
    }

    // Allow reusing the port immediately after closing
    int reuse = 1;
    (void)::setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    // Create the address structure for listening on all interfaces
    sockaddr_in local_address{};
    local_address.sin_family = AF_INET;
    local_address.sin_port = htons(port);
    local_address.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind the socket to the address
    if(::bind(socket_fd, reinterpret_cast<sockaddr*>(&local_address), sizeof(local_address)) < 0)
    {
        std::perror("bind");
        ::close(socket_fd);
        return -1;
    }

    return socket_fd;

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


    // Optinal : Allow port override
    if(arg_count == 2)
    {
        const long port_loc = std::strtol(arg_vector[1], nullptr, 10); // 10 - Decimal, 16 - Hex, 02 - Binary
        if(port_loc > 0 && port_loc < 65535)
        {
            port = static_cast<uint16_t>(port_loc);
        }
    }

    // Create the UDP Receiver
    const int receiver_fd = create_and_bind_udp_receiver_socket(port);

    // Chekc if the UDP receiver is started successfully
    if(receiver_fd < 0)
    {
        std::fprintf(stderr,"Failed to start the UDP receiver : %u \n", port);
        return 1;
    }

    std::fprintf("UDP console receiver started listening on 0.0.0.0: %u\n", port);
    std::fprintf("Press Ctrl + C to stop. \n\n");

    while(1)
    {
        // Infinite receiver loop
    }

    ::close(receiver_fd);

    return 0;
}

