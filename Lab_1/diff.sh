#!/bin/bash

# Advanced Test
for i in $(seq 0 9)
do
    echo "*** user${i}.txt ***"
    diff user${i}.txt Ans.txt -y --suppress-common-lines
done

# Loading Test
for i in $(seq 1 999)
do
    echo "*** user${i}Load.txt ***"
    diff user${i}Load.txt user0Load.txt -y --suppress-common-lines
done