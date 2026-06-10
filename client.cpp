#include <iostream>
#include <winsock2.h>

using namespace std;

int main()
{

    WSADATA wsa;
    // Initialize Winsock for this process.
    // MAKEWORD(2, 2) requests Winsock version 2.2.
    // wsa receives implementation-specific information.
    WSAStartup(MAKEWORD(2, 2), &wsa);

    int sockfd;
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
        cout << "Socket creation failed\n";
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
        cout << "Connection failed - Server rejected or unreachable\n";
        // You can:
        // 1. Retry after a delay
        // 2. Try a different server
        // 3. Exit gracefully
        closesocket(sockfd);
        return 1;
    }

    cout << "Connected to server\n";

    while (true)
    {
        // Send message
        cout << "Client: ";
        cin.getline(buffer, sizeof(buffer));

        // send(socket, buffer, len, flags)
        //   socket: connected socket descriptor.
        //   buffer: data to send.
        //   len: number of bytes to send.
        //   flags: normally 0; other values include MSG_OOB, MSG_DONTROUTE.
        send(sockfd, buffer, strlen(buffer) + 1, 0);

        // Exit condition
        if (string(buffer) == "q")
            break;

        // Receive response
        // recv(socket, buffer, len, flags)
        //   socket: connected socket descriptor.
        //   buffer: storage for received bytes.
        //   len: maximum bytes to read.
        //   flags: normally 0; other values include MSG_PEEK, MSG_WAITALL.
        // Returns number of bytes received, 0 if connection closed, SOCKET_ERROR on error.
        recv(sockfd, buffer, sizeof(buffer), 0);

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