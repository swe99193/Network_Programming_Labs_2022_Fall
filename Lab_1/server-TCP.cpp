// Server side C/C++ program to demonstrate Socket
// programming
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define PORT 8080



int main(int argc, char const* argv[])
{
    int server_fd, new_fd;
    struct sockaddr_in server_addr, client_addr;
    int valread;
    char* msg = "Welcome to Game 1A2B";
    char buffer[1024] = { 0 };
    int opt = 1;
    int addrlen = sizeof(server_addr);

    // Creating socket file descriptor
    // SOCK_STREAM: TCP(reliable, connection oriented)
    // SOCK_DGRAM: UDP(unreliable, connectionless)
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    // configure the options: reuse of address and port
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // bind socket
    bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));

    // listen to incoming request
    listen(server_fd, 3);
    printf("Server listening..\n");

    // accept request in queue
    int len = sizeof(client_addr);
    new_fd = accept(server_fd, (struct sockaddr*)&client_addr, (socklen_t*)&len);
    
    valread = read(new_fd, buffer, 1024);
    printf("%s\n", buffer);
    send(new_fd, msg, strlen(msg), 0);

    // closing the connected socket
    close(new_fd);
    // closing the listening socket
    shutdown(server_fd, SHUT_RDWR);
    return 0;
}

