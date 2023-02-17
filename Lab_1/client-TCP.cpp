// Client side C/C++ program to demonstrate Socket
// programming
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>


char* ADDR  = "127.0.0.1";
int PORT = 8080;

int main(int argc, char const* argv[])
{
    int sockfd, valread;
    struct sockaddr_in server_addr;
    char* msg = "New connection.";
    char buffer[1024] = { 0 };

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ADDR);
    server_addr.sin_port = htons(PORT);


    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }
    send(sockfd, msg, strlen(msg), 0);
    valread = read(sockfd, buffer, 1024);
    printf("%s\n", buffer);

    // closing the connected socket
    close(sockfd);
    // printf("%d\n", sockfd);
    return 0;
}

