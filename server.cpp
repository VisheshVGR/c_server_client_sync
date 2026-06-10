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
    // MAKEWORD(2, 2) requests version 2.2 of the Winsock API.
    // The wsa variable receives implementation-specific details and status.
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        cout << "WSAStartup failed: " << WSAGetLastError() << "\n";
        return 1;
    }

    SOCKET server_fd, client_fd;

    sockaddr_in server_addr, client_addr;

    char clientbuffer[1024];
    char serverbuffer[1024];

    // Create a socket.
    // socket(int af, int type, int protocol)
    //   af:
    //     AF_INET  - IPv4
    //     AF_INET6 - IPv6
    //     AF_UNIX  - local UNIX domain sockets (Windows has limited support)
    //   type:
    //     SOCK_STREAM - reliable, connection-oriented TCP.
    //     SOCK_DGRAM  - connectionless UDP.
    //     SOCK_RAW    - raw IP packets (requires privileges).
    //   protocol:
    //     IPPROTO_TCP - TCP (often 0 with SOCK_STREAM).
    //     IPPROTO_UDP - UDP (often 0 with SOCK_DGRAM).
    //     0           - choose default protocol for given family/type.
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == INVALID_SOCKET)
    {
        cout << "Socket creation failed\n";
        return 1;
    }

    // Server configuration.
    // sockaddr_in fields:
    //   sin_family: address family, here AF_INET for IPv4.
    //   sin_port: port number in network byte order.
    //   sin_addr.s_addr: IPv4 address in network byte order.
    // Use htons() to convert host-byte-order port to network byte order.
    // Use htonl() to convert host-byte-order IP to network byte order.
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // bind(socket, address, address_len)
    //   socket: the socket descriptor to attach the address to.
    //   address: pointer to sockaddr structure with IP and port.
    //   address_len: size of the address structure.
    // Return value: 0 on success, SOCKET_ERROR on failure.
    int bind_result = bind(server_fd,
                              (sockaddr *)&server_addr,
                              sizeof(server_addr));
    if (bind_result == SOCKET_ERROR)
    {
        cout << "Bind failed: " << WSAGetLastError() << "\n";
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    // listen(socket, backlog)
    //   socket: listening socket descriptor.
    //   backlog: maximum number of pending connections in queue.
    //   Common values: 1, 5, 10, SOMAXCONN. Larger backlog allows more waiting clients.
    // Return value: 0 on success, SOCKET_ERROR on failure.
    int listen_result = listen(server_fd, 1);
    if (listen_result == SOCKET_ERROR)
    {
        cout << "Listen failed: " << WSAGetLastError() << "\n";
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    int addrlen = sizeof(client_addr);

    while (true)
    {
        cout << "Waiting for client...\n";

        // accept(listen_socket, client_address, address_len)
        //   listen_socket: socket created, bound, and set to listen().
        //   client_address: pointer to sockaddr structure filled with peer info.
        //   address_len: pointer to size of client_address buffer.
        // On success, returns a new socket descriptor for the accepted client.
        // On failure, returns INVALID_SOCKET.
        // The original server_fd stays open to accept more clients.
        client_fd = accept(server_fd,
                           (sockaddr *)&client_addr,
                           &addrlen);

        if (client_fd == INVALID_SOCKET)
        {
            cout << "Accept failed: " << WSAGetLastError() << "\n";
            break;
        }

        cout << "Client connected (Timeout 50sec)\n";

        // CHAT WITH CLIENT

        while (true)
        {
            // Receive from client
            int timeout = 50000; // 50 seconds in milliseconds
            // Set receive timeout for this client socket.
            // SOL_SOCKET: socket-level option.
            // SO_RCVTIMEO: timeout for recv().
            // (char *)&timeout: pointer to the timeout value.
            // sizeof(timeout): length of the timeout value.
            setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));

            // Receive data from the client.
            // n is the number of bytes received, or SOCKET_ERROR on failure.
            if (!recv_message(client_fd, clientbuffer, sizeof(clientbuffer)))
            {
                cout << "ERROR: Failed to receive client data or timeout\n";
                break;
            }

            if (string(clientbuffer) == "q")
            {
                cout << "Client EXITED\n";
                break;
            }

            cout << "\nClient: " << clientbuffer << endl;

            // Send message
            cout << "Server: ";
            cin.getline(serverbuffer, sizeof(serverbuffer));

            if (string(serverbuffer) == "q")
            {
                // Exit condition for the server side.
                cout << "EXITED\n";

                if (!send_message(client_fd, "SERVER EXITED"))
                    cout << "ERROR: Failed to send exit notice\n";

                break;
            }
            else if (!send_message(client_fd, serverbuffer))
            {
                cout << "ERROR: Failed to send response\n";
                break;
            }
        }

        // Exit condition
        if (string(serverbuffer) == "q")
            break;
    }

    // Close the client and server sockets.
    closesocket(client_fd);
    closesocket(server_fd);

    // Clean up Winsock resources for this process.
    WSACleanup();

    return 0;
}