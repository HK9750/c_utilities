#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include<stdio.h>
#include <sys/socket.h>
#include<stdlib.h>
#include <netinet/in.h>
#include <unistd.h>

int main() {
    // socket(AF_INET,SOCK_STREAM,0) creates a TCP socket using IPv4
    // AF_INET: Address family for IPv4, SOCK_STREAM: Type for TCP, 0: Default protocol (TCP for SOCK_STREAM)
    int socket_file_descriptor = socket(AF_INET,SOCK_STREAM,0);
    if(socket_file_descriptor < 0) {
        perror("socketfd is less than 0");
        exit(-1);
    }
    int opt = 1;
    // setsockopt() is used to set options on the socket. Here, we set SO_REUSEADDR to allow the socket to be quickly reused after it is closed.
    // SOL_SOCKET: Level for socket options, SO_REUSEADDR: Option to allow reuse of local addresses, &opt: Pointer to the option value, sizeof(opt): Size of the option value
    // option represents the value to be set for the option. In this case, opt is set to 1, which means that the SO_REUSEADDR option will be enabled for the socket.
    setsockopt(socket_file_descriptor,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

    // sockaddr_in is a structure that contains an internet address. It is used to specify the address and port for the socket.
    struct sockaddr_in addr;
    // sin_family: Address family (AF_INET for IPv4), sin_port: Port number (htons converts it to network byte order), sin_addr.s_addr: IP address (INADDR_ANY allows the server to accept connections on any of the host's IP addresses)
    addr.sin_family = AF_INET;
    // htons() converts the port number from host byte order to network byte order, which is necessary for the socket to function correctly across different platforms.
    addr.sin_port = htons(8080);
    // INADDR_ANY is a constant that allows the server to accept connections on any of the host's IP addresses. It is typically used for servers that want to listen on all available interfaces.
    addr.sin_addr.s_addr = INADDR_ANY;

    // bind() associates the socket with the specified address and port. It takes the socket file descriptor, a pointer to the sockaddr structure, and the size of the sockaddr structure as arguments. If the bind operation fails, it prints an error message, closes the socket, and exits the program.
    if(bind(socket_file_descriptor,(struct sockaddr*)&addr,sizeof(addr)) < 0) {
        perror("bind failed");
        close(socket_file_descriptor);
        exit(-1);
    }
    // listen() marks the socket as a passive socket that will be used to accept incoming connection requests. It takes the socket file descriptor and the backlog (the maximum number of pending connections) as arguments. If the listen operation fails, it prints an error message, closes the socket, and exits the program.
    // if the pending connection queue is full, the server will reject new connection attempts until there is space in the queue.
    if(listen(socket_file_descriptor, 10) < 0) {
        perror("listen failed");
        close(socket_file_descriptor);
        exit(-1);
    }

    printf("server listening on port 8080\n");

    while(1) {
        // accept() is used to accept incoming connection requests. It takes the socket file descriptor, a pointer to a sockaddr structure to store the client's address, and a pointer to a socklen_t variable that specifies the size of the sockaddr structure. If the accept operation fails, it prints an error message and continues to the next iteration of the loop.
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_fd = accept(socket_file_descriptor, (struct sockaddr*)&client_addr, &client_addr_len);
        if(client_fd < 0) {
            perror("accept failed");
            continue;
        }
        printf("one nigga is here");
        // printf("client connected from IP: %s, port: %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        // Here, you would typically create a new thread or process to handle the client connection using client_fd. For simplicity, we just close the client socket immediately.
        // close(client_fd);
    }

    // close() is used to close the socket file descriptor when the server is shutting down. This releases the resources associated with the socket.
    close(socket_file_descriptor);
    return 0;
}