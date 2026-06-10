#include <iostream>
#include <winsock2.h>
#include <cstring>
#include <cstdint>

using namespace std;

static bool send_all(SOCKET sock, const char *data, int length)
{
    int total_sent = 0;
    while (total_sent < length)
    {
        int sent = send(sock, data + total_sent, length - total_sent, 0);
        if (sent == SOCKET_ERROR)
            return false;
        total_sent += sent;
    }
    return true;
}

static bool recv_all(SOCKET sock, char *buffer, int length)
{
    int total_received = 0;
    while (total_received < length)
    {
        int received = recv(sock, buffer + total_received, length - total_received, 0);
        if (received <= 0)
            return false;
        total_received += received;
    }
    return true;
}

static bool send_message(SOCKET sock, const char *message)
{
    uint32_t payload_length = static_cast<uint32_t>(strlen(message));
    uint32_t network_length = htonl(payload_length);
    if (!send_all(sock, reinterpret_cast<const char *>(&network_length), sizeof(network_length)))
        return false;
    return payload_length == 0 || send_all(sock, message, payload_length);
}

static bool recv_message(SOCKET sock, char *buffer, int buffer_size)
{
    uint32_t network_length = 0;
    if (!recv_all(sock, reinterpret_cast<char *>(&network_length), sizeof(network_length)))
        return false;

    uint32_t payload_length = ntohl(network_length);
    if (payload_length >= static_cast<uint32_t>(buffer_size))
        return false;

    if (payload_length > 0 && !recv_all(sock, buffer, payload_length))
        return false;

    buffer[payload_length] = '\0';
    return true;
}

int main()
{

    WSADATA wsa;
    // Initialize Winsock for this process.
    // MAKEWORD(2, 2) requests Winsock version 2.2.
    // wsa receives implementation-specific information.
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        cout << "WSAStartup failed: " << WSAGetLastError() << "\n";
        return 1;
    }

    SOCKET sockfd;
    sockaddr_in server_addr;
    char buffer[1024];

    // Create a TCP/IP socket.
    // socket(af, type, protocol)
    //   af:
    //     AF_INET  - IPv4
    //     AF_INET6 - IPv6
    //   type:
    //     SOCK_STREAM - reliable TCP stream
    //     SOCK_DGRAM  - connectionless UDP datagrams
    //     SOCK_RAW    - raw IP packets (requires privileges)
    //   protocol:
    //     IPPROTO_TCP - TCP protocol
    //     IPPROTO_UDP - UDP protocol
    //     0           - default protocol for af/type
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == INVALID_SOCKET)
    {
        cout << "Socket creation failed: " << WSAGetLastError() << "\n";
        WSACleanup();
        return 1;
    }

    // Server configuration.
    // sockaddr_in fields:
    //   sin_family: address family (AF_INET for IPv4).
    //   sin_port: port number in network byte order.
    //   sin_addr.s_addr: IPv4 address in network byte order.
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // connect(socket, address, address_len)
    //   socket: connected socket descriptor.
    //   address: pointer to server address structure.
    //   address_len: size of the address structure.
    // Returns 0 on success, SOCKET_ERROR on failure.
    int result = connect(sockfd,
                         (sockaddr *)&server_addr,
                         sizeof(server_addr));

    if (result == SOCKET_ERROR)
    {
        cout << "Connection failed - Server rejected or unreachable: " << WSAGetLastError() << "\n";
        // You can:
        // 1. Retry after a delay
        // 2. Try a different server
        // 3. Exit gracefully
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }

    cout << "Connected to server\n";

    while (true)
    {
        // Send message
        cout << "Client: ";
        cin.getline(buffer, sizeof(buffer));

        if (!send_message(sockfd, buffer))
        {
            cout << "Failed to send message\n";
            break;
        }

        // Exit condition
        if (string(buffer) == "q")
            break;

        // Receive response
        if (!recv_message(sockfd, buffer, sizeof(buffer)))
        {
            cout << "Failed to receive response\n";
            break;
        }

        cout << "\nServer: " << buffer << endl;

        // Exit condition
        if (string(buffer) == "SERVER EXITED")
            break;
    }

    // Close the client socket.
    closesocket(sockfd);

    // Clean up Winsock resources.
    WSACleanup();

    return 0;
}