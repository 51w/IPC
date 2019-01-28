#include <stdio.h>
#include <unistd.h>

#include "stream.h"
#include "librtspserver.hh"
#include "rtmpstream.h"

extern int BroadCast_UDP();
int main()
{
	printf("======================\n");
	BroadCast_UDP();
	IPC_main();
	//RtspServer();
	//rtmpstream_run();
	srsrtmp_run();
	
	usleep(2*1000*1000);
	set_bitrate(600);
	usleep(200*1000*1000);
	
	IPC_exit();
	//RtspServerExit();
	//rtmpstream_exit();
	srsrtmp_exit();
	
	printf("======================\n");
}