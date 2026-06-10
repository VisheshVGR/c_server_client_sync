# Server-Client Sync

A simple synchronous TCP socket communication application built with C++ and Windows Sockets (Winsock2).

## Overview

This project demonstrates basic client-server communication using TCP/IP sockets on Windows. The server listens for incoming connections and exchanges messages with connected clients in a chat-like interface.

## Files

### `server.cpp`
- **Purpose**: Runs the TCP server that listens for client connections
- **Details**:
  - Binds to `localhost:8080`
  - Accepts one client connection at a time
  - Implements a 50-second receive timeout per client
  - Exchanges messages with the connected client
  - Supports graceful shutdown with "q" command
- **Key Functions**:
  - `socket()` - Creates a TCP socket
  - `bind()` - Binds socket to port 8080
  - `listen()` - Waits for incoming connections
  - `accept()` - Accepts client connections
  - `recv()` / `send()` - Communicates with client

### `client.cpp`
- **Purpose**: Connects to the server and sends/receives messages
- **Details**:
  - Connects to server at `127.0.0.1:8080`
  - Sends user input messages to the server
  - Receives and displays server responses
  - Closes connection when user types "q"
- **Key Functions**:
  - `socket()` - Creates a TCP socket
  - `connect()` - Connects to server
  - `send()` / `recv()` - Communicates with server

## How to Use

### Prerequisites
- Windows OS (uses Winsock2)
- C++ compiler (Visual Studio, MinGW, etc.)
- Winsock2 library (`ws2_32.lib`)

### Compilation

**With Visual Studio or cl.exe**:
```bash
cl server.cpp ws2_32.lib
cl client.cpp ws2_32.lib
```

**With g++/MinGW**:
```bash
g++ server.cpp -o server.exe -lws2_32
g++ client.cpp -o client.exe -lws2_32
```

### Running the Application

1. **Start the server** (in one terminal):
   ```bash
   server.exe
   ```
   Output: `Waiting for client...`

2. **Start the client** (in another terminal):
   ```bash
   client.exe
   ```
   Output: `Connected to server`

3. **Chat**:
   - Type a message on the client side and press Enter
   - The server will receive and display the message
   - Type a response on the server side and press Enter
   - The client receives and displays the response
   - Continue until either side types `q` to exit

### Example Session

**Server Terminal**:
```
Waiting for client...
Client connected (Timeout 50sec)

Client: Hello
Server: Hi there!

Client: How are you?
Server: I'm doing great!

Client: q
Client EXITED
```

**Client Terminal**:
```
Connected to server
Client: Hello
Server: Hi there!
Client: How are you?
Server: I'm doing great!
Client: q
```

## Technical Details

- **Protocol**: TCP/IP (Winsock2)
- **Connection**: Localhost (`127.0.0.1`) on port `8080`
- **Buffer Size**: 1024 bytes for messages
- **Timeout**: Server has a 50-second receive timeout per client
- **Message Format**: Null-terminated strings

## Limitations

- Only accepts one client at a time
- Synchronous operation (no multi-threading)
- Windows-only (uses Winsock2)
- No error recovery or message validation
