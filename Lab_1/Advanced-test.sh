#!/bin/bash

make 

SERVER_SESSION="server"
CLIENT_SESSION="client"


IP=$1
PORT=$2
SLEEP_TIME=2

if [ -n "`tmux ls | grep $SERVER_SESSION`" ]; then
  tmux kill-session -t $SERVER_SESSION
fi

if [ -n "`tmux ls | grep $CLIENT_SESSION`" ]; then
  tmux kill-session -t $CLIENT_SESSION
fi

# Create the server
tmux new-session -d -s $SERVER_SESSION
tmux set remain-on-exit on

tmux send-keys -t $SERVER_SESSION "./server $PORT" ENTER


# Create the clients
tmux new-session -d -s $CLIENT_SESSION
tmux set remain-on-exit on

# create windows
for i in $(seq 1 9)
do
  tmux new-window
done

sleep 2

# Test 1: UDP (Concurrency)
echo -e "\nTest 1: UDP (Concurrency)\n"

echo -e "connection\n"
for i in $(seq 0 9)
do
    tmux send-keys -t ${i}.0 "./client $IP $PORT >> user${i}.txt" Enter
done
sleep $SLEEP_TIME

echo -e "game-rule\n"
for i in $(seq 0 9)
do
    tmux send-keys -t ${i}.0 "game-rule" Enter
done
sleep $SLEEP_TIME

echo -e "game-rule\n"
for i in $(seq 0 9)
do
    tmux send-keys -t ${i}.0 "game-rule" Enter
done
sleep $SLEEP_TIME

echo -e "exit\n"
for i in $(seq 0 9)
do
    tmux send-keys -t ${i}.0 "exit" Enter          
done
sleep $SLEEP_TIME


# Test 2: Register, Login, logout (Concurency)
echo -e "\nTest 2: Register, Login, logout (Concurency)\n"

echo -e "connection\n"
for i in $(seq 0 9)
do
    tmux send-keys -t ${i}.0 "./client $IP $PORT >> user${i}.txt" Enter
done
sleep $SLEEP_TIME

echo -e "register - wrong usage\n"
for i in $(seq 0 9)
do
    tmux send-keys -t ${i}.0 "register" Enter
done
sleep $SLEEP_TIME

echo -e "login - wrong usage\n"
for i in $(seq 0 9)
do
    tmux send-keys -t ${i}.0 "login" Enter
done
sleep $SLEEP_TIME

echo -e "logout - without login\n"
for i in $(seq 0 9)
do
    tmux send-keys -t ${i}.0 "logout" Enter
done
sleep $SLEEP_TIME

echo -e "login - without regsiter\n"
for i in $(seq 0 9)
do
    tmux send-keys -t ${i}.0 "login ${i} ${i}" Enter
done
sleep $SLEEP_TIME

echo -e "register\n"
for i in $(seq 0 9)
do
    tmux send-keys -t ${i}.0 "register ${i} ${i} ${i}" Enter
done
sleep $SLEEP_TIME

echo -e "register - duplicate user\n"
for i in $(seq 0 9)
do
    tmux send-keys -t ${i}.0 "register ${i} DuplicateUser ${i}" Enter
done
sleep $SLEEP_TIME

echo -e "register - duplicate email\n"
for i in $(seq 0 9)
do
    tmux send-keys -t ${i}.0 "register DuplicateEmail ${i} ${i}" Enter
done
sleep $SLEEP_TIME

echo -e "login - wrong PWD\n"
for i in $(seq 0 9)
do
    tmux send-keys -t ${i}.0 "login ${i} WrongPWD" Enter
done
sleep $SLEEP_TIME

echo -e "login - wrong user\n"
for i in $(seq 0 9)
do
    tmux send-keys -t ${i}.0 "login WrongUSER ${i}" Enter
done
sleep $SLEEP_TIME

echo -e "login\n"
for i in $(seq 0 9)
do
    tmux send-keys -t ${i}.0 "login ${i} ${i}" Enter
done
sleep $SLEEP_TIME

echo -e "login - without logout\n"
for i in $(seq 0 9)
do
    tmux send-keys -t ${i}.0 "login ${i} ${i}" Enter
done
sleep $SLEEP_TIME

echo -e "logout\n"
for i in $(seq 0 9)
do
    tmux send-keys -t ${i}.0 "logout" Enter
done
sleep $SLEEP_TIME

echo -e "logout - without login\n"
for i in $(seq 0 9)
do
    tmux send-keys -t ${i}.0 "logout" Enter
done
sleep $SLEEP_TIME

echo -e "exit\n"
for i in $(seq 0 9)
do
    tmux send-keys -t ${i}.0 "exit" Enter          
done
sleep $SLEEP_TIME


# Test 3: UDP & TCP mixed (Concurrency)
echo -e "\nTest 3: UDP & TCP mixed (Concurrency)\n"

echo -e "connection\n"
for i in $(seq 0 9)
do
    tmux send-keys -t ${i}.0 "./client $IP $PORT >> user${i}.txt" Enter
done
sleep $SLEEP_TIME

echo -e "game-rule\n"
for i in $(seq 0 9)
do
    tmux send-keys -t ${i}.0 "game-rule" Enter
done
sleep $SLEEP_TIME

echo -e "login\n"
for i in $(seq 0 9)
do
    tmux send-keys -t ${i}.0 "login ${i} ${i}" Enter
done
sleep $SLEEP_TIME

echo -e "game-rule\n"
for i in $(seq 0 9)
do
    tmux send-keys -t ${i}.0 "game-rule" Enter
done
sleep $SLEEP_TIME

echo -e "logout\n"
for i in $(seq 0 9)
do
    tmux send-keys -t ${i}.0 "logout" Enter
done
sleep $SLEEP_TIME

echo -e "game-rule\n"
for i in $(seq 0 9)
do
    tmux send-keys -t ${i}.0 "game-rule" Enter
done
sleep $SLEEP_TIME

echo -e "login - exit without logout\n"
for i in $(seq 0 9)
do
    tmux send-keys -t ${i}.0 "login ${i} ${i}" Enter
done
sleep $SLEEP_TIME

echo -e "exit\n"
for i in $(seq 0 9)
do
    tmux send-keys -t ${i}.0 "exit" Enter          
done
sleep $SLEEP_TIME



# Test 4: Game (Concurency)
echo -e "\nTest 4: Game (Concurency)\n"

echo -e "connection\n"
for i in $(seq 0 9)
do
    tmux send-keys -t ${i}.0 "./client $IP $PORT >> user${i}.txt" Enter
done
sleep $SLEEP_TIME

echo -e "start-game - not login\n"
for i in $(seq 0 9)
do
    tmux send-keys -t ${i}.0 "start-game AK28" Enter
done
sleep $SLEEP_TIME

echo -e "start-game - not login\n"
for i in $(seq 0 9)
do
    tmux send-keys -t ${i}.0 "start-game" Enter
done
sleep $SLEEP_TIME

echo -e "login\n"
for i in $(seq 0 9)
do
    tmux send-keys -t ${i}.0 "login ${i} ${i}" Enter
done
sleep $SLEEP_TIME

echo -e "start-game - wrong usage\n"
for i in $(seq 0 9)
do
    tmux send-keys -t ${i}.0 "start-game AK28" Enter
done
sleep $SLEEP_TIME


# echo -e "start-game - lose\n"
# for i in $(seq 0 9)
# do
#     tmux send-keys -t ${i}.0 "start-game 4222" Enter
#     sleep 1
#     tmux send-keys -t ${i}.0 "1444" Enter
#     sleep 1
#     tmux send-keys -t ${i}.0 "2444" Enter
#     sleep 1
#     tmux send-keys -t ${i}.0 "4000" Enter
#     sleep 1
#     tmux send-keys -t ${i}.0 "4002" Enter
#     sleep 1
#     tmux send-keys -t ${i}.0 "2a" Enter
#     sleep 1
#     tmux send-keys -t ${i}.0 "aa44" Enter
#     sleep 1
#     tmux send-keys -t ${i}.0 "1234567890abcdefg" Enter
#     sleep 1
#     tmux send-keys -t ${i}.0 "9999" Enter
#     sleep 1
# done
# sleep $SLEEP_TIME

echo -e "start-game - lose\n"
for i in $(seq 0 9)
do
    tmux send-keys -t ${i}.0 "start-game 4222" Enter
done
sleep $SLEEP_TIME

for i in $(seq 0 9)
do
    tmux send-keys -t ${i}.0 "1444" Enter
done
sleep $SLEEP_TIME

for i in $(seq 0 9)
do
    tmux send-keys -t ${i}.0 "2444" Enter
done
sleep $SLEEP_TIME

for i in $(seq 0 9)
do
    tmux send-keys -t ${i}.0 "4000" Enter
done
sleep $SLEEP_TIME

for i in $(seq 0 9)
do
    tmux send-keys -t ${i}.0 "4002" Enter
done
sleep $SLEEP_TIME

for i in $(seq 0 9)
do
    tmux send-keys -t ${i}.0 "2a" Enter
done
sleep $SLEEP_TIME

for i in $(seq 0 9)
do
    tmux send-keys -t ${i}.0 "aa44" Enter
done
sleep $SLEEP_TIME

for i in $(seq 0 9)
do
    tmux send-keys -t ${i}.0 "1234567890abcdefg" Enter
done
sleep $SLEEP_TIME

for i in $(seq 0 9)
do
    tmux send-keys -t ${i}.0 "9999" Enter
done
sleep $SLEEP_TIME


echo -e "start-game - win\n"
for i in $(seq 0 9)
do
    tmux send-keys -t ${i}.0 "start-game 4222" Enter
done
sleep $SLEEP_TIME

for i in $(seq 0 9)
do
    tmux send-keys -t ${i}.0 "4222" Enter
done
sleep $SLEEP_TIME

echo -e "logout\n"
for i in $(seq 0 9)
do
    tmux send-keys -t ${i}.0 "logout" Enter
done
sleep $SLEEP_TIME

echo -e "exit\n"
for i in $(seq 0 9)
do
    tmux send-keys -t ${i}.0 "exit" Enter          
    sleep $SLEEP_TIME
done


# If you want to see how your program is running on tmux,
# you can comment out these two commands and just use
# "tmux attach -t np_demo_server" or "tmux attach -t np_demo_client"
# to check server and client respectively
# tmux kill-session -t $SERVER_SESSION
# tmux kill-session -t $CLIENT_SESSION


# Delete the executable files
# rm server
# rm client
# rm user*
# rm test_*