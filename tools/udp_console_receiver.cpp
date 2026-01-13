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
    return 0;
}

