#CROSS_COMPILE = arm-hisiv300-linux-
CXX   = $(CROSS_COMPILE)g++
CC    = $(CROSS_COMPILE)gcc

all:
	$(CXX) udp264.cpp -std=c++11 -o udp
	$(CXX) main.cpp RtpConnect.cpp -std=c++11 -o connect
