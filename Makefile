
CC=g++ -std=c++11 -O2

all: corrected standard 
corrected: DR_BN_fsc 
standard: DR_BN_std

DR_BN_fsc: DR_BN_fsc.cpp
	$(CC) -o DR_BN_fsc DR_BN_fsc.cpp

DR_BN_std: DR_BN_std.cpp
	$(CC) -o DR_BN_std DR_BN_std.cpp
