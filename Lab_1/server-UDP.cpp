// Server side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> // socket APIs
#include <arpa/inet.h> //inet_addr
#include <netinet/in.h> //sockaddr_in
	
#define PORT	 8080

	
// Driver code
int main() {
	int sockfd;
	char buffer[1024] = {0};
	char *hello = "Hello from server!";
	struct sockaddr_in server_addr, client_addr;
		
	// Creating socket file descriptor
    // SOCK_DGRAM: UDP(unreliable, connectionless)
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		
	server_addr.sin_family = AF_INET; // IPv4
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(PORT);
		
	// Bind the socket with the server address
	bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	int len = sizeof(client_addr);

    recvfrom(sockfd, buffer, 1024, MSG_WAITALL, ( struct sockaddr *) &client_addr, (socklen_t *)&len);

	printf("Client : %s\n", buffer);
	sendto(sockfd, hello, strlen(hello), MSG_CONFIRM, ( struct sockaddr *) &client_addr, (socklen_t)len);
	printf("Hello message from Server.\n");
		
	return 0;
}

