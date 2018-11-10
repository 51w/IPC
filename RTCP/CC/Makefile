#CROSS_COMPILE = arm-hisiv300-linux-
CXX   = $(CROSS_COMPILE)g++
CC    = $(CROSS_COMPILE)gcc

all:
	$(CXX) client.cpp -o client
	$(CXX) udp.cpp -std=c++11 -o udp
