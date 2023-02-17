// Client side
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <string>
#include <sstream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace std;

char* ADDR = "127.0.0.1";
int PORT = 8080;

void play_game(int TCP_fd) {
    while (1) {
        char buffer[1024] = { 0 };
        string guess;
        getline(cin, guess);

        char req[1024] = {0};  // request
        for (int i = 0; i < guess.size(); i++) // string to char array
            req[i] = guess[i];

        send(TCP_fd, req, strlen(req), 0);
        read(TCP_fd, buffer, 1024);
        string response = buffer;

        cout << response;

        if(response == "You got the answer!\n" || response == "You lose the game!\n")
            break;  // endGame
    }
}


int main(int argc, char* argv[])
{
    ADDR = argv[1];
    PORT = atoi(argv[2]);

    int TCP_fd = -1, UDP_fd = -1;
    struct sockaddr_in server_addr;

    UDP_fd = socket(AF_INET, SOCK_DGRAM, 0);

    // configure server Address & Port 
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ADDR);
    server_addr.sin_port = htons(PORT);

    TCP_fd = socket(AF_INET, SOCK_STREAM, 0);
    connect(TCP_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));

    bool init = true;   // initial connection
    while (1) {
        char buffer[1024] = { 0 };
        string input, cmd;

        if (init) {
            input = "New connection.\n";
        } else {
                                        // cout << "Enter command:" <<endl;
            getline(cin, input);

            // parse command
            stringstream s(input);
            s >> cmd;
        }

        char req[1024] = {0};  // request
        for (int i = 0; i < input.size(); i++) // string to char array
            req[i] = input[i];

        // UDP
        if (cmd == "register" || cmd == "game-rule") {
            int len;
            sendto(UDP_fd, req, strlen(req), MSG_CONFIRM, (const struct sockaddr *) &server_addr, sizeof(server_addr));        
            recvfrom(UDP_fd, buffer, 1024, MSG_WAITALL, (struct sockaddr *) &server_addr, (socklen_t *)&len);
            printf("%s", buffer);
        } 
        // TCP
        else if(cmd == "login" || cmd == "logout" || cmd == "start-game" || cmd == "exit" || init) {
            send(TCP_fd, req, strlen(req), 0);
            read(TCP_fd, buffer, 1024);
            string response = buffer;

            cout << response;

            if (cmd == "start-game" && response == "Please typing a 4-digit number:\n") {
                play_game(TCP_fd);
            }
            else if (cmd == "exit"){
                close(TCP_fd);
                break;
            } 
            init = false;
        }
        
    }
    
    close(UDP_fd);  // close UDP socket

    return 0;
}

