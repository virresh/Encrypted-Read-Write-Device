#!/bin/bash
set -e
cd drivers
make
sudo insmod encdevdriver.ko
make clean
cd ../testing
make
echo -e "\n\n"
./encoder infile efile keyfile
sudo rmmod encdevdriver
make clean