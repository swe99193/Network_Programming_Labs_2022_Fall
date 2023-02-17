#!/bin/bash

# This not a real script, just a step-by-step document

# compilation tools
sudo yum groupinstall "Development Tools" -y
sudo yum install libcurl-devel openssl-devel libuuid-devel pulseaudio-libs-devel -y


# install cmake
wget https://cmake.org/files/v3.21/cmake-3.21.6-linux-x86_64.sh
bash cmake-3.21.6-linux-x86_64.sh 
## set path variables, add to ~/.bash_profile for persistency
# nano ~/.bash_profile
export PATH=$PATH:/home/ec2-user/cmake-3.21.6-linux-x86_64/bin/


# C++ AWS SDK source
mkdir aws-sdk
cd aws-sdk
git clone --recurse-submodules https://github.com/aws/aws-sdk-cpp

# follow this: https://docs.aws.amazon.com/sdk-for-cpp/v1/developer-guide/setup-linux.html
mkdir sdk_build
cd sdk_build

cmake ../aws-sdk-cpp -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=/usr/local/ -DCMAKE_INSTALL_PREFIX=/usr/local/ -DBUILD_ONLY="s3;iam;sts"

make
sudo make install
# move back to home
cd ~

# SDK sample code
git clone https://github.com/awsdocs/aws-doc-sdk-examples.git
cmake ../aws-doc-sdk-examples/cpp/example_code/s3

# add s3_examples.h
mkdir -p include/awsdoc/s3/
nano include/awsdoc/s3/s3_examples.h

# add CMakeLists.txt, server, build.sh
nano CMakeLists.txt
nano server.cpp
nano build.sh

# scp test scripts & testcases
scp -i ~/.ssh/labsuser.pem -r testcases testcases_correct client.py demo.py ec2-user@0.0.0.0:~/

# bulid server
bash build.sh

# test part1
python3 demo.py --server-ip 0.0.0.0
# python3 demo.py --server-ip 0.0.0.0 -t list_invitations_sample

# test part3
~/build/server
python3 demo_part3.py --server-ips ip1 ip2 ip3

34.232.50.150 44.201.226.177 54.160.155.194