CC  = gcc
CXX = g++

INC = -I./include
LIB = -L./shared 
LIBFF = -lavcodec -lavutil -lpthread
LIBCV = -lopencv_core -lopencv_highgui -lopencv_imgproc


all: main clenO


clenO:
	rm *.o

main.o : main.cpp
	$(CXX) -c  $< -o $@  $(INC) $(LIB) $(LIBCV) $(LIBFF) 
	
dec264.o : dec264.c
	$(CC) -c  $< -o $@  $(INC) $(LIB) $(LIBCV) $(LIBFF) 

main: main.o dec264.o
	$(CXX) $^ -o $@  $(INC) $(LIB) $(LIBCV) $(LIBFF) 
	
clean:
	-rm -rf main.o dec264.o main