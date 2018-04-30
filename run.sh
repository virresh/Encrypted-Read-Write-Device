#!/bin/bash
set -e
cd drivers
make
sudo insmod encdevdriver.ko 
sudo insmod decdevdriver.ko
make clean
cd ../testing
make
echo -e "\n\n"
./encoder infile efile keyfile
./decoder efile outfile keyfile
sudo rmmod encdevdriver
sudo rmmod decdevdriver
make clean