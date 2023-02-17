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

int PORT = 8888;

class userinfo {
    public:
        string username;    // hash key
        string email;
        string password;
        bool isOnline;
        long long roomid;

        // dummy default constructor for map
        userinfo(){}

        /* register */ 
        userinfo(string username, string email, string password) {
            this->username = username;
            this->email = email;
            this->password = password;
            this->isOnline = false;
            this->roomid = -1;
        }
};

class room {
    public:
        long long id;   // hash key
        long long code;
        bool isPublic;
        bool isGaming;
        string manager; // username
        vector<string> members; // a list of usernames
        int step;
        int rounds;
        string answer;

        // dummy default constructor
        room(){}

        /* public room */ 
        room(long long id, string manager) {
            this->id = id;
            this->isPublic = true;
            this->isGaming = false;
            this->manager = manager;
            this->members = vector<string>();
            this->step = 0;
            this->rounds = 0;
        }

        /* private room */ 
        room(long long id, string manager, long long code) {
            this->id = id;
            this->code = code;
            this->isPublic = false;
            this->isGaming = false;
            this->manager = manager;
            this->members = vector<string>();
            this->step = 0;
            this->rounds = 0;
        }
};

class invitation {
    public:
        long long roomid;
        long long code;
        string inviter; // username
        string receiver; // username
        bool valid;

        // dummy default constructor 
        invitation(){}
 
        invitation(long long roomid, long long code, string inviter, string receiver) {
            this->roomid = roomid;
            this->code = code;
            this->inviter = inviter;
            this->receiver = receiver;
            this->valid = true;
        }
};


unordered_set<string> username;
unordered_set<string> email;
unordered_map<string, userinfo> UserInfos; // username -> userinfo

unordered_map<string, int> sessionDB; // username -> socket

unordered_map<long long, room> Rooms; // room id -> roominfo
unordered_set<long long> RoomPool; // room id

// (deprecated)
// unordered_map<string, vector<invitation> > RECVinvitations; // username -> invitations
// unordered_map<string, vector<invitation> > SENTinvitations; // username -> invitations
vector<invitation> Invitations;

pthread_mutex_t mutexExit = PTHREAD_MUTEX_INITIALIZER;
// pthread_mutex_t mutexSessionDB = PTHREAD_MUTEX_INITIALIZER;


void TCPsend(int socket, string response) {
    char res[1024] = {0};  // response
    for (int i = 0; i < response.size(); i++) // string to char array
        res[i] = response[i];
    send(socket, res, strlen(res), 0);
}

string TCPread(int socket) {
    char buffer[1024] = { 0 };
    read(socket, buffer, 1024);
    string input = buffer;  // convert char array to c++ string
    return input;
}

void broadcast(string cur_usr, long long roomid, string response){
    for (auto& usr: Rooms[roomid].members){
        if (usr != cur_usr){    // skip yourself
            int socket = sessionDB[usr];
            TCPsend(socket, response);
        }
    }
}


string registerHandler(vector<string> param){
    if (param.size() != 3)
        return "Usage: register <username> <email> <password>\n";

    string usr = param[0];
    string mail = param[1];
    string pwd = param[2];

    if (username.find(usr) != username.end() \
            || email.find(mail) != email.end())
        return "Username or Email is already used\n";
    else {      // register a new account
        username.insert(usr);
        email.insert(mail);
        UserInfos[usr] = userinfo(usr, mail, pwd);
        return "Register Successfully\n";
    }
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
            
            pthread_mutex_lock( &mutexExit );
        if (cmd == "register"){
            result = registerHandler(param);
        }
        else if (cmd == "list") {
            if (param[0] == "users") {
                result += "List Users\n";
                if (username.empty()){
                    result += "No Users\n";
                } else {
                    // sort username alphapetically
                    vector<string> usrs;
                    for (auto& [key, value]: UserInfos)
                        usrs.push_back(key);
                    sort(usrs.begin(), usrs.end());

                    int i = 1;
                    for (auto& x: usrs) {
                        result += to_string(i) + ". " + x;
                        result += "<" + UserInfos[x].email + "> ";
                        if (UserInfos[x].isOnline)
                            result += "Online";
                        else
                            result += "Offline";
                        result += "\n";
                        i += 1;
                    }

                }
            } else if (param[0] == "rooms") {
                result += "List Game Rooms\n";
                if (RoomPool.empty()){
                    result += "No Rooms\n";
                } else {
                    // sort room id
                    vector<long long> RoomIds;
                    for (auto& [key, value]: Rooms)
                        RoomIds.push_back(key);
                    sort(RoomIds.begin(), RoomIds.end());

                    int i = 1;
                    for (auto& x: RoomIds) {
                        result += to_string(i) + ". ";

                        if (Rooms[x].isPublic)
                            result += "(Public) ";
                        else
                            result += "(Private) ";

                        result += "Game Room " + to_string(x);

                        if (Rooms[x].isGaming)
                            result += " has started playing";
                        else
                            result += " is open for players";
                        result += "\n";
                        i += 1;
                    }
                }
            }
        }
            pthread_mutex_unlock( &mutexExit );

        char res[1024] = {0};  // response
        for (int i = 0; i < result.size(); i++) // string to char array
            res[i] = result[i];
        
        sendto(server_fd, res, strlen(res), MSG_CONFIRM, ( struct sockaddr *) &client_addr, (socklen_t)len);
    }

    pthread_exit(NULL);
}


string loginHandler(vector<string> param, string &cur_usr, int socket){
    if (param.size() != 2)
        return "Usage: login <username> <password>\n";

    string usr = param[0];
    string pwd = param[1];

    if (username.find(usr) == username.end())
        return "Username does not exist\n";
    else if (cur_usr != "")
        return "You already logged in as " + cur_usr + "\n";
    else if (sessionDB.find(usr) != sessionDB.end())
        return "Someone already logged in as " + usr + "\n";
    else if ( pwd != UserInfos[usr].password) {
        return "Wrong password\n";
    } else{
        cur_usr = usr;  // login
        UserInfos[cur_usr].isOnline = true;
        sessionDB[cur_usr] = socket;
        return "Welcome, " + cur_usr + "\n";
    }
}


string logoutHandler(string &cur_usr, int socket){
    if (cur_usr == "")
        return "You are not logged in\n";
    else if (UserInfos[cur_usr].roomid != -1)
        return "You are already in game room " + to_string(UserInfos[cur_usr].roomid) + ", please leave game room\n";
    else {
        string res = "Goodbye, " + cur_usr + "\n";
        UserInfos[cur_usr].isOnline = false;
        sessionDB.erase(cur_usr);
        cur_usr = "";   // logout
        return res;
    }
}


string createRoom(vector<string> param, string cur_usr) {
    long long roomid = stoll(param[2], NULL);

    if (cur_usr == "")
        return "You are not logged in\n";
    else if (UserInfos[cur_usr].roomid != -1) {
        return "You are already in game room " + to_string(UserInfos[cur_usr].roomid) + ", please leave game room\n";
    } else if (RoomPool.find(roomid) != RoomPool.end()) {
        return "Game room ID is used, choose another one\n";
    } else {
        string result = "You create ";

        UserInfos[cur_usr].roomid = roomid;

        if (param[0] == "private") {
            long long code = stoll(param[3]);
            Rooms[roomid] = room(roomid, cur_usr, code);
            result += "private";
        }
        else {
            Rooms[roomid] = room(roomid, cur_usr);
            result += "public";
        }
        Rooms[roomid].members.push_back(cur_usr);  // add yourself
        RoomPool.insert(roomid);

        result += " game room " + param[2] + "\n";
        return result;
    }

}


string joinRoom(vector<string> param, string cur_usr) {
    long long roomid = stoll(param[1], NULL);

    if (cur_usr == "")
        return "You are not logged in\n";
    else if (UserInfos[cur_usr].roomid != -1) {
        return "You are already in game room " + to_string(UserInfos[cur_usr].roomid) + ", please leave game room\n";
    } else if (RoomPool.find(roomid) == RoomPool.end()){
        return "Game room " + to_string(roomid) + " is not exist\n";
    } else if (!Rooms[roomid].isPublic) {
        return "Game room is private, please join game by invitation code\n";
    } else if (Rooms[roomid].isGaming) {
        return "Game has started, you can't join now\n";
    } else {
        // join room
        UserInfos[cur_usr].roomid = roomid;
        Rooms[roomid].members.push_back(cur_usr);

        string response = "Welcome, " + cur_usr + " to game!\n";

        // broadcast
        broadcast(cur_usr, roomid, response);

        return "You join game room " + to_string(roomid) + "\n";
    }
}

        // for (auto& inv: SENTinvitations[cur_usr]) {
        //     string receiver = inv.receiver;
        //     inv.valid = false;
        //     for (auto& recvinv: RECVinvitations[receiver]) {
        //         if(recvinv.inviter == cur_usr)
        //             recvinv.valid = false;
        //     }
        // }
            // for (auto& recvinv: RECVinvitations["Coffee"]) {
            //     if(recvinv.inviter == cur_usr)
            //         recvinv.valid = false;
            // }
string leaveRoom(string cur_usr, bool mute) {
    if (cur_usr == "")
        return "You are not logged in\n";
    else if (UserInfos[cur_usr].roomid == -1) {
        return "You did not join any game room\n";
    } else {
        // FIXME: set invalid every sent invitation
        // for (auto& inv: Invitations) {
        //     if(inv.inviter == cur_usr){
        //         inv.roomid = -1;
        //         inv.code = NULL;
        //         inv.valid = false;
        //     }
        // }

        // leave room
        long long roomid = UserInfos[cur_usr].roomid;

        UserInfos[cur_usr].roomid = -1;
        
        // 1. room manager
        if (cur_usr == Rooms[roomid].manager) {
            Rooms[roomid].isGaming = false; // end game

            // delete this room, remove all members
            for (auto& user: Rooms[roomid].members){
                UserInfos[user].roomid = -1;
            }
            RoomPool.erase(roomid);

            // FIXME: remove all invitaions to your room
            for (auto& inv: Invitations) {
                if(inv.roomid == roomid){
                    inv.roomid = -1;
                    inv.code = NULL;
                    inv.valid = false;
                }
            }

            string response = "Game room manager leave game room " + to_string(roomid) + ", you are forced to leave too\n";

            // broadcast
            if (!mute) {
                broadcast(cur_usr, roomid, response);
            }

            return "You leave game room " + to_string(roomid) + "\n";

        } 
        // 2. is gaming
        else if (Rooms[roomid].isGaming){
            Rooms[roomid].isGaming = false; // end game

            Rooms[roomid].members.erase(std::remove(Rooms[roomid].members.begin(), Rooms[roomid].members.end(), cur_usr), Rooms[roomid].members.end()); // vector
            
            string response = cur_usr + " leave game room " + to_string(roomid) + ", game ends\n";

            // broadcast
            if (!mute) {
                broadcast(cur_usr, roomid, response);
            }

            return "You leave game room " + to_string(roomid) + ", game ends\n";
        }
        // 3. not gaming
        else {
            Rooms[roomid].isGaming = false; // end game
            
            // remove youself from this room
            // Rooms[roomid].members.erase(cur_usr); // unordered_set
            Rooms[roomid].members.erase(std::remove(Rooms[roomid].members.begin(), Rooms[roomid].members.end(), cur_usr), Rooms[roomid].members.end()); // vector
            // https://stackoverflow.com/questions/3385229/c-erase-vector-element-by-value-rather-than-by-position
            // vec.erase(std::remove(vec.begin(), vec.end(), 8), vec.end());

            string response = cur_usr + " leave game room " + to_string(roomid) + "\n";

            // broadcast
            if (!mute) {
                broadcast(cur_usr, roomid, response);
            }

            return "You leave game room " + to_string(roomid) + "\n";
        }
    }

}


string invite(vector<string> param, string cur_usr){
    string email = param[0];
    long long roomid = UserInfos[cur_usr].roomid;   // current room

    if (cur_usr == "")
        return "You are not logged in\n";
    else if (roomid == -1) {
        return "You did not join any game room\n";
    } else if (Rooms[roomid].manager != cur_usr) {
        return "You are not private game room manager\n";
    } else {
        long long code = Rooms[roomid].code;
        string receiver;
        
        // query username
        for (auto& [username, userinfo]: UserInfos)
            if (userinfo.email == email){
                receiver = username;
                break;
            }

        if(!UserInfos[receiver].isOnline) {
            return "Invitee not logged in\n";
        } else {
            int socket = sessionDB[receiver];
            
            string response = "You receive invitation from " + cur_usr + "<" + UserInfos[cur_usr].email + ">\n";

            TCPsend(socket, response);

            // POST database
            invitation inv(roomid, code, cur_usr, receiver);
            // RECVinvitations[receiver].push_back(inv);
            // SENTinvitations[cur_usr].push_back(inv);
            
            // overwrite existing invitation if any
            int N = Invitations.size();
            int i = 0;
            for (i=0; i<N; ++i) {
                if (Invitations[i].inviter == cur_usr && Invitations[i].receiver == receiver) {
                    Invitations[i].roomid = roomid;
                    Invitations[i].code = code;
                    Invitations[i].valid = true;
                    break;
                }
            }
            // insert new entry
            if (i >= N)
                Invitations.push_back(inv);
            
            return "You send invitation to " + receiver + "<" + UserInfos[receiver].email + ">\n";
        }
    }

}

        // vector<invitation> tmp = RECVinvitations[cur_usr];
        // string list;

        // // sort on roomid
        // sort(tmp.begin(), tmp.end(), [](invitation& A, invitation& B){return A.roomid < B.roomid;});

        // int i = 0;
        // for (auto& recvinv: tmp) 
        //     if (recvinv.valid) {
        //         i += 1;
                
        //         string inviter = recvinv.inviter;
        //         string email = UserInfos[inviter].email;

        //         list += to_string(i) + ". ";
        //         list += inviter + "<" + email + ">";
        //         list += " invite you to join game room " + to_string(recvinv.roomid) + ", invitation code is " + to_string(recvinv.code) + "\n";
        //     }
string listInvitation(string cur_usr){
    if (cur_usr == "")
        return "You are not logged in\n";
    else {
        string list;

        // sort on roomid
        // https://stackoverflow.com/questions/5122804/how-to-sort-with-a-lambda
        sort(Invitations.begin(), Invitations.end(), [](invitation& A, invitation& B){return A.roomid < B.roomid;});

        int i = 0;
        for (auto& inv: Invitations) 
            if (inv.valid && inv.receiver == cur_usr) {
                i += 1;
                
                string inviter = inv.inviter;
                string email = UserInfos[inviter].email;

                list += to_string(i) + ". ";
                list += inviter + "<" + email + ">";
                list += " invite you to join game room " + to_string(inv.roomid) + ", invitation code is " + to_string(inv.code) + "\n";
            }

        if (i == 0)
            return "List invitations\nNo Invitations\n";
        else
            return "List invitations\n" + list;
    }
}


        // for (auto& recvinv: RECVinvitations[cur_usr]) {
        //     string inviter = recvinv.inviter;
        //     if (recvinv.valid && UserInfos[inviter].email == email) {
        //         is_valid = true;
        //         if (recvinv.code == code){
        //             code_correct = true;
        //             roomid = recvinv.roomid;
        //         }
        //     }
        // }
string acceptInvitation(vector<string> param, string cur_usr) {
    string email = param[0];
    long long code = stoll(param[1], NULL);

    if (cur_usr == "")
        return "You are not logged in\n";
    else if (UserInfos[cur_usr].roomid != -1) {
        return "You are already in game room " + to_string(UserInfos[cur_usr].roomid) + ", please leave game room\n";
    } else {
        bool is_valid = false;  // invitation exist
        bool code_correct = false;  // code check
        long long roomid = NULL;
        for (auto& inv: Invitations) {
            string inviter = inv.inviter;
            string receiver = inv.receiver;
            if (inv.valid && UserInfos[inviter].email == email && receiver == cur_usr) {
                is_valid = true;
                if (inv.code == code){
                    code_correct = true;
                    roomid = inv.roomid;
                }
            }
        }

        if (!is_valid){
            return "Invitation not exist\n";
        } else if (!code_correct) {
            return "Your invitation code is incorrect\n";
        } else if (Rooms[roomid].isGaming) {
            return "Game has started, you can't join now\n";
        } else {
            // join room
            UserInfos[cur_usr].roomid = roomid;
            Rooms[roomid].members.push_back(cur_usr);

            string response = "Welcome, " + cur_usr + " to game!\n";

            // broadcast
            broadcast(cur_usr, roomid, response);

            return "You join game room " + to_string(roomid) + "\n";
        }
    }
}


string startGame(vector<string> param, string cur_usr) {
    int rounds = stoi(param[1]);
    string answer;

    if (cur_usr == "")
        return "You are not logged in\n";
    else if (UserInfos[cur_usr].roomid == -1)
        return "You did not join any game room\n";

    long long roomid = UserInfos[cur_usr].roomid;

    if (Rooms[roomid].manager != cur_usr)
        return "You are not game room manager, you can't start game\n";
    else if (Rooms[roomid].isGaming)
        return "Game has started, you can't start again\n";
    
    if (param.size() == 3){
        answer = param[2];
        if (answer.length() != 4 || ! all_of(answer.begin(), answer.end(), ::isdigit))
            return "Please enter 4 digit number with leading zero\n";
    }
    else {
        for(int i=0; i<4; i++)
            answer += to_string(rand()%10);
                                // cout << ans << endl;
    }

    // init game state
    Rooms[roomid].isGaming = true;
    Rooms[roomid].step = 0;
    Rooms[roomid].rounds = rounds;
    Rooms[roomid].answer = answer;

    string response = "Game start! Current player is " + Rooms[roomid].members[0] + "\n";

    // broadcast
    broadcast(cur_usr, roomid, response);

    return response;
}


string guessHint(string guess, string answer) {
    int a = 0, b = 0;   // hints
    unordered_map<char, int> count_unused;

    for(int i=0; i<4; i++){
        if(guess[i] == answer[i])
            a += 1;
        else {
            b += 1;
            count_unused[answer[i]] += 1;
            count_unused[guess[i]] -= 1;
        }
    }
    
    // adjust B (delete counts that use too many digits)
    for(int i=0; i<10; i++){
        char ch = '0' + i;
        if (count_unused[ch] < 0)
            b += count_unused[ch];  // decrease
    }
    return to_string(a) + "A" + to_string(b) + "B";
}

string guess(vector<string> param, string cur_usr, int socket) {
    if (cur_usr == "")
        return "You are not logged in\n";
    else if (UserInfos[cur_usr].roomid == -1)
        return "You did not join any game room\n";

    long long roomid = UserInfos[cur_usr].roomid;
    int step = Rooms[roomid].step;
    int rounds = Rooms[roomid].rounds;
    string manager = Rooms[roomid].manager;
    string player = Rooms[roomid].members[step];
    string answer = Rooms[roomid].answer;
    int TOTAL = Rooms[roomid].members.size();

    if (!Rooms[roomid].isGaming)
        if(cur_usr == manager)
            return "You are game room manager, please start game first\n";
        else
            return "Game has not started yet\n";
    else if (cur_usr != player)
        return "Please wait..., current player is " + player + "\n";


    string your_guess = param[0];
    int N = your_guess.length();

    if (N != 4)
        return "Please enter 4 digit number with leading zero\n";
    else if (your_guess == answer) {
        // correct
        string response = cur_usr + " guess \'" + your_guess + "\' and got Bingo!!! " + cur_usr + " wins the game, game ends\n";

        broadcast(cur_usr, roomid, response);

        Rooms[roomid].isGaming = false;
        return response;
    }
    else {
        // incorrect
        string hint = guessHint(your_guess, answer); // 3A2B
        string response = cur_usr + " guess \'" + your_guess + "\' and got \'" + hint + "\'\n";

        step += 1;
        if (step == TOTAL){
            step -= TOTAL;
            rounds -= 1;
        }
        Rooms[roomid].step = step;
        Rooms[roomid].rounds = rounds;

        // game ends
        if(rounds == 0){
            response += "Game ends, no one wins\n";
            Rooms[roomid].isGaming = false;
        }

        broadcast(cur_usr, roomid, response);
        return response;
    }
}


/* TCP session */
void* TCPsession(void* args) {
    int socket = *((int *) args);
    string cur_usr;    // if login, it will become non-empty
                                // cout << "**TCP Server session start**" <<endl;
    while(1) {
        string input = TCPread(socket);
        string result;
        vector<string> param;

                                // cout << "client: "<<input << endl;
        // parse parameters
        stringstream s(input);
        string cmd, word;
        s >> cmd;   // command
        while (s >> word)
            param.push_back(word);  // parameters

            pthread_mutex_lock( &mutexExit );
        if (cmd == "login")
            result = loginHandler(param, cur_usr, socket);
        else if (cmd == "logout")
            result = logoutHandler(cur_usr, socket);
        else if (cmd == "create") {
            result = createRoom(param, cur_usr);
        }
        else if (cmd == "join") {
            result = joinRoom(param, cur_usr);
        }
        else if (cmd == "leave") {
            result = leaveRoom(cur_usr, false);
        }
        else if (cmd == "invite") {
            result = invite(param, cur_usr);
        }
        else if (cmd == "list") {
            result = listInvitation(cur_usr);
        }
        else if (cmd == "accept") {
            result = acceptInvitation(param, cur_usr);
        }
        else if (cmd == "start") {
            result = startGame(param, cur_usr);
        }
        else if (cmd == "guess") {
            result = guess(param, cur_usr, socket);
        }
        else {
            leaveRoom(cur_usr, true);   // auto leave room
            UserInfos[cur_usr].isOnline = false;
            UserInfos[cur_usr].roomid = -1;
            sessionDB.erase(cur_usr); // auto logout
            pthread_mutex_unlock( &mutexExit );
            break;
        }
            pthread_mutex_unlock( &mutexExit );


        TCPsend(socket, result);
    }
                                // cout << "**TCP Server session end**" <<endl;
    // closing the connected socket
    close(socket);
    pthread_exit(NULL);
}


int main(int argc, char* argv[])
{
    // PORT = atoi(argv[1]);

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

    char *hostip = inet_ntoa(server_addr.sin_addr);
    printf("Host ip: %s\n", hostip);

    // listen to incoming request
    listen(server_fd, 30);
                    cout << "**TCP server listening**" <<endl;

    int i = 0;
    while (1){
        // accept request in queue
        int len = sizeof(client_addr);
        new_fd[i] = accept(server_fd, (struct sockaddr*)&client_addr, (socklen_t *)&len);
                            // cout << "socket id:" << new_fd[i] << endl;
        // enter a new session thread
        pthread_create(&tid[i], NULL, TCPsession, &new_fd[i]);
                                        // cout << "** i: " << i << endl;
        i = (i + 1) % 100;
    }

    // closing the listening socket
    shutdown(server_fd, SHUT_RDWR);
    return 0;
}

