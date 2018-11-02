CC = arm-hisiv300-linux-g++

all:
	$(CC) facenet.cpp mobilefacenet.cpp mtcnn.cpp -I../../3516/include -L../../3516/lib -L../../3516/install/lib -lncnn -std=c++11 -lopencv_core -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc -o main