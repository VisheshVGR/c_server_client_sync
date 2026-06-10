#include <iostream>
#include <winsock2.h>

using namespace std;

int main()
{
    WSADATA wsa;
    // Initialize Winsock for this process.
    // MAKEWORD(2, 2) requests version 2.2 of the Winsock API.
    // The wsa variable receives implementation-specific details and status.
    WSAStartup(MAKEWORD(2, 2), &wsa);

    int server_fd, client_fd;

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
    bind(server_fd,
         (sockaddr *)&server_addr,
         sizeof(server_addr));

    // listen(socket, backlog)
    //   socket: listening socket descriptor.
    //   backlog: maximum number of pending connections in queue.
    //   Common values: 1, 5, 10, SOMAXCONN. Larger backlog allows more waiting clients.
    // Return value: 0 on success, SOCKET_ERROR on failure.
    listen(server_fd, 1);

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
            int n = recv(client_fd, clientbuffer, sizeof(clientbuffer), 0);

            if (n == 0)
            {
                cout << "ERROR: Client disconnected\n";
                break;
            }
            else if (n == SOCKET_ERROR)
            {
                cout << "ERROR: Timeout or error\n"; // Timeout occurred
                break;
            }

            // Ensure null-termination within bounds
            clientbuffer[min(n, (int)sizeof(clientbuffer) - 1)] = '\0';

            // CLIENT Exit condition
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

                // Send a final notification to the client.
                // strlen(...)+1 includes the terminating null byte.
                send(client_fd, "SERVER EXITED", strlen("SERVER EXITED") + 1, 0);

                break;
            }
            else
            {
                // Send the typed server response back to the client.
                send(client_fd, serverbuffer, strlen(serverbuffer) + 1, 0);
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