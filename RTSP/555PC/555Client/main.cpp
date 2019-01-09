#include <stdio.h>
#include <unistd.h>
#include "player.h"

int main()
{
	char *ip = "192.168.1.2";
	init(ip);
	
	usleep(10* 1000*1000);
	
	shutDown();
	
	return 0;
}