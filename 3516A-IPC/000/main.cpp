#include <iostream>
#include <unistd.h>

#include "stream.h"
#include "librtspserver.hh"

int main()
{
	std::cout << "=====" << std::endl;
	
	IPC_main();
	RtspServer();
	
	usleep(20*1000*1000);
	//set_bitrate(500);
	
	IPC_exit();
	RtspServerExit();
	
	std::cout << "=============" << std::endl;
}