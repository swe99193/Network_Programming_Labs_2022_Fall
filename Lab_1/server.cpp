// Server side
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <string.h>
#include <vector>
#include <queue>  // pthread id queue
#include <unordered_set>
#include <unordered_map>
#include <algorithm>    // all_of()
#include <string>
#include <sys/socket.h> // socket APIs
#include <sys/types.h>
#include <netinet/in.h> //sockaddr_in
#include <arpa/inet.h>  //inet_addr
#include <unistd.h> // system call
#include <cstdlib>  // rand
#include <pthread.h>


using namespace std;

int PORT = 8080;
unordered_set<string> username; // username (set)
unordered_set<string> email; // email (set)
unordered_map<string, string> password; // username -> password (hash)
unordered_set<string> sessionDB; // sessionDB, username as element (set)

pthread_mutex_t mutexRegister = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexSessionDB = PTHREAD_MUTEX_INITIALIZER;


string registerHandler(vector<string> param){
    if (param.size() != 3)
        return "Usage: register <username> <email> <password>\n";

    string usr = param[0];
    string mail = param[1];
    string pwd = param[2];

    if (username.find(usr) != username.end())
        return "Username is already used.\n";
    else if (email.find(mail) != email.end())
        return "Email is already used.\n";
    else {      // register a new account
        username.insert(usr);
        email.insert(mail);
        password[usr] = pwd;
        return "Register successfully.\n";
    }
}


string loginHandler(vector<string> param, string &usr){
    if (param.size() != 2)
        return "Usage: login <username> <password>\n";
    // if (usr != "")  // loggedin state
    //     return "Please logout first.\n";

    string tmp_usr = param[0];
    string pwd = param[1];

    if (sessionDB.find(tmp_usr) != sessionDB.end()) // prevent multiple logins from different sockets
        return "Please logout first.\n";
    else if (username.find(tmp_usr) == username.end())
        return "Username not found.\n";
    else if ( pwd != password[tmp_usr]) {
        return "Password not correct.\n";
    } else{
        usr = tmp_usr;
        sessionDB.insert(usr);
        return "Welcome, " + usr + ".\n";
    }
}


string logoutHandler(vector<string> param, string usr){
    if (sessionDB.find(usr) == sessionDB.end())
        return "Please login first.\n";
    else {
        sessionDB.erase(usr);
        return "Bye, " + usr + ".\n";
    }
}


string gameRuleHandler(){
    // adjacent string literals are concatenated by the compiler
    return "1. Each question is a 4-digit secret number.\n"
    "2. After each guess, you will get a hint with the following information:\n"
    "2.1 The number of \"A\", which are digits in the guess that are in the correct position.\n"
    "2.2 The number of \"B\", which are digits in the guess that are in the answer but are in the wrong position.\n"
    "The hint will be formatted as \"xAyB\".\n"
    "3. 5 chances for each question.\n";
}


string gameHandler(vector<string>& param, string usr, bool& gameStarted, string& ans){
    if (sessionDB.find(usr) == sessionDB.end())
        return "Please login first.\n";

    if (param.size() > 1)
        return "Usage: start-game <4-digit number>\n";

    // set answer
    if (param.size() == 1){
        if (param[0].length() != 4 || ! all_of(param[0].begin(), param[0].end(), ::isdigit))
            return "Usage: start-game <4-digit number>\n";
        ans = param[0];
    }
    else {
        ans = "";
        for(int i=0; i<4; i++)
            ans += to_string(rand()%10);
                                // cout << ans << endl;
    }

    gameStarted = true;
    return "Please typing a 4-digit number:\n";
}


void play_game(const int socket_fd, const string ans){
    int countFail = 0;

    while (1) {
        char buffer[1024] = { 0 };
        read(socket_fd, buffer, 1024);
        string guess = buffer;  // convert char array to c++ string
        string result;
        bool endGame = false;
        int N = guess.length();

        // all_of(): check isdigits
        if(N != 4 || ! all_of(guess.begin(), guess.end(), ::isdigit))
            result = "Your guess should be a 4-digit number.\n";
        else if (guess == ans) {
            result = "You got the answer!\n";
            endGame = true;
        }
        else {
            countFail++;
            int a = 0, b = 0;   // hints
            unordered_map<char, int> count_unused;

            for(int i=0; i<4; i++){
                if(guess[i] == ans[i])
                    a += 1;
                else {
                    b += 1;
                    count_unused[ans[i]] += 1;
                    count_unused[guess[i]] -= 1;
                }
            }
            
            // adjust B (delete counts that use too many digits)
            for(int i=0; i<10; i++){
                char ch = '0' + i;
                if (count_unused[ch] < 0)
                    b += count_unused[ch];  // decrease
            }
            result = to_string(a) + "A" + to_string(b) + "B\n";
        }

        if (countFail == 5){
            result += "You lose the game!\n";
            endGame = true;
        }

        char res[1024] = {0};  // response
        for (int i = 0; i < result.size(); i++) // string to char array
            res[i] = result[i];
        send(socket_fd, res, strlen(res), 0);

        if (endGame)  break;  // exit
    }
}

/* TCP session */
void* session(void* args){
    int socket_fd = *((int *) args);
    string usr;    // if login, it will become non-empty
    bool leave = false; // end socket
    bool gameStarted = false;
    
                                // cout << "**TCP Server session start**" <<endl;
    while(1) {
        char buffer[1024] = { 0 };
        read(socket_fd, buffer, 1024);
        string input = buffer;  // convert char array to c++ string
        string result;
        string ans; // game answer
        vector<string> param;

                                // cout << "client: "<<input << endl;
        // parse parameters
        stringstream s(input);
        string cmd, word;
        s >> cmd;   // command
        while (s >> word)
            param.push_back(word);  // parameters

        if (cmd == "login" || cmd == "logout" || cmd == "exit"){
            pthread_mutex_lock( &mutexSessionDB );
            if (cmd == "login")
                result = loginHandler(param, usr);
            else if (cmd == "logout")
                result = logoutHandler(param, usr);
            else if (cmd == "exit") {
                leave = true;
                sessionDB.erase(usr); // auto logout
            }
            pthread_mutex_unlock( &mutexSessionDB );
        }
        else if (cmd == "start-game")
            result = gameHandler(param, usr, gameStarted, ans);
        else { // initial connection
            cout << input;
            result = "*****Welcome to Game 1A2B*****\n";
        }

        char res[1024] = {0};  // response
        for (int i = 0; i < result.size(); i++) // string to char array
            res[i] = result[i];
        
        send(socket_fd, res, strlen(res), 0);
        
        if (gameStarted) {  // start game
            play_game(socket_fd, ans);
            gameStarted = false;
        }

        if (leave)  break;  // exit
    }
                                // cout << "**TCP Server session end**" <<endl;
    // closing the connected socket
    close(socket_fd);
    pthread_exit(NULL);
}


/********** UDP server **********/
void* UDP_Server(void* args) {
    int server_fd;
	struct sockaddr_in server_addr, client_addr;
		
	// Creating socket fd
    // SOCK_DGRAM: UDP(unreliable, connectionless)
	server_fd = socket(AF_INET, SOCK_DGRAM, 0);
		
	server_addr.sin_family = AF_INET; // IPv4
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(PORT);
		
	// Bind the socket with the server address
	bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    while (1){
    	char buffer[1024] = {0};
        int len = sizeof(client_addr);
                    // cout << "**UDP Server waiting msg**" <<endl;
        recvfrom(server_fd, buffer, 1024, MSG_WAITALL, ( struct sockaddr *) &client_addr, (socklen_t *)&len);

        string input = buffer;  // convert char array to c++ string
        string result;
        vector<string> param;

                                    // cout << "client: "<<input << endl;

        // parse parameters
        stringstream s(input);
        string cmd, word;
        s >> cmd;   // command
        while (s >> word)
            param.push_back(word);  // parameters
        
        if (cmd == "register"){
            pthread_mutex_lock( &mutexRegister );
            result = registerHandler(param);
            pthread_mutex_unlock( &mutexRegister );
        }
        else if (cmd == "game-rule")
            result = gameRuleHandler();

        char res[1024] = {0};  // response
        for (int i = 0; i < result.size(); i++) // string to char array
            res[i] = result[i];
        
        // send(socket_fd, res, strlen(res), 0);
        sendto(server_fd, res, strlen(res), MSG_CONFIRM, ( struct sockaddr *) &client_addr, (socklen_t)len);
    }

    pthread_exit(NULL);
}

int main(int argc, char* argv[])
{
    PORT = atoi(argv[1]);

    pthread_t tid[100];  // TCP thread pool
    // queue<int> Usedtid; // thread in use

    pthread_t UDPtid;  // UDP server
    pthread_create(&UDPtid, NULL, UDP_Server, NULL);


    /********** TCP server **********/
    int server_fd;
    int new_fd[100]; // client socket fd
    struct sockaddr_in server_addr, client_addr;
    int opt = 1;
    int addrlen = sizeof(server_addr);

    // Creating socket fd
    // SOCK_STREAM: TCP(reliable, connection oriented)
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
  
    // configure the options: reuse of address and port
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    // configure server Address & Port 
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // bind socket
    bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));

    // listen to incoming request
    listen(server_fd, 30);
                    // cout << "**TCP server listening**" <<endl;

    int i = 0;
    while (1){
        // accept request in queue
        int len = sizeof(client_addr);
        new_fd[i] = accept(server_fd, (struct sockaddr*)&client_addr, (socklen_t *)&len);
        
        // enter a new session thread
        pthread_create(&tid[i], NULL, session, &new_fd[i]);
                                        // cout << "** i: " << i << endl;
        i = (i + 1) % 100;
    }

    // closing the listening socket
    shutdown(server_fd, SHUT_RDWR);
}

