#include <thread>
#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <unistd.h>

void snedFrame();

int main(int agrc, char **argv)
{
	int clients = 0;
	
	std::cout << "=======00=====" << std::endl;
	std::thread t1(snedFrame);
	std::cout << "=======11=====" << std::endl;
	
	usleep(10*1000);
	std::cout << "=======5=====" << std::endl;
	//getchar();
}

void snedFrame()
{
	std::cout << "=======2=====" << std::endl;
}