// Client side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

char* ADDR  = "127.0.0.1";
int PORT = 8080;

// Driver code
int main() {
    int sockfd;
    char buffer[1024] = { 0 };
    char *hello = "Hello from client~";
    struct sockaddr_in server_addr;

    // Creating socket file descriptor
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ADDR);
    server_addr.sin_port = htons(PORT);
        
    int len;
        
    sendto(sockfd, hello, strlen(hello), MSG_CONFIRM, (const struct sockaddr *) &server_addr, sizeof(server_addr));
    printf("Hello message from client.\n");
            
    recvfrom(sockfd, buffer, 1024, MSG_WAITALL, (struct sockaddr *) &server_addr, (socklen_t *)&len);
    printf("Server : %s\n", buffer);

    close(sockfd);
    return 0;
}

