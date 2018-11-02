CROSS_COMPILE = arm-hisiv300-linux-
CXX   = $(CROSS_COMPILE)g++
CC    = $(CROSS_COMPILE)gcc

all:
	$(CXX) bufpool.cc -shared -fPIC -lpthread -o libbufpool.so
