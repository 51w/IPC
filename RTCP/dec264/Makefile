CC  = gcc
CXX = g++

INC = -I./include
LIB = -L./shared 
LIBFF = -lavcodec -lavutil -lpthread
LIBCV = -lopencv_core -lopencv_highgui -lopencv_imgproc


all: client clenO


clenO:
	rm *.o

client.o : client.cpp
	$(CXX) -c  $< -o $@  $(INC) $(LIB) $(LIBCV) $(LIBFF) 
	
dec264.o : dec264.c
	$(CC) -c  $< -o $@  $(INC) $(LIB) $(LIBCV) $(LIBFF) 

client: client.o dec264.o
	$(CXX) $^ -o $@  $(INC) $(LIB) $(LIBCV) $(LIBFF) 
	
clean:
	-rm -rf client.o dec264.o client