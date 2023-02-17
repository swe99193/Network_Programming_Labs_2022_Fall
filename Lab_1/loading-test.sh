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
for i in $(seq 1 999)
do
  tmux new-window
done

sleep 2

# Test 5: many TCP clients
echo -e "\nTest 5: many TCP clients\n"

echo -e "connection\n"
for i in $(seq 0 999)
do
    tmux send-keys -t ${i}.0 "./client $IP $PORT >> user${i}Load.txt" Enter
done
sleep $SLEEP_TIME

echo -e "exit\n"
for i in $(seq 0 999)
do
    tmux send-keys -t ${i}.0 "exit" Enter          
done
sleep $SLEEP_TIME


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