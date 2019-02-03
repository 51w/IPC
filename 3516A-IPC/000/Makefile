CROSS_COMPILE = arm-hisiv300-linux-
CXX   = $(CROSS_COMPILE)g++
CC    = $(CROSS_COMPILE)gcc

INC = -I./bufpool2 -I./stream -I./yuv2rgb -I./rtmpdump
LIB = -L./bufpool2 -L./stream -L./yuv2rgb -L./rtmpdump

LIB += -lbufpool2 -lyuv2rgb -lstream -lrtsp -lrtmp
LIB += -lpthread  -lstdc++

SRC = main.c broadcast.c rtmpstream.c autonet.c

SRC += srs/srs_librtmp.cpp srs/srsrtmp.c
INC += -I./srs 

all:
	$(CC) $(SRC) -o main $(INC) $(LIB)
