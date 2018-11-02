CROSS_COMPILE = arm-hisiv300-linux-
CXX   = $(CROSS_COMPILE)g++
CC    = $(CROSS_COMPILE)gcc
AR    = $(CROSS_COMPILE)ar

INC += -I../../../3516/include -I../3516/include
LIB += -L../../../3516/lib  -L../3516 -lncnn
#LIB += -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_imgcodecs

all:
	$(CXX) -c main.c mtcnn.cpp $(INC) $(LIB) -std=c++11
	$(AR) rc libmtcnn.a *.o
