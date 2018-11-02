#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "librtspserver.hh"
#include "bufpool.h"

pthread_t gs_VencPid;
void *rtspserver(void* p);

int main()
{
    int port = 8554;
    pthread_create(&gs_VencPid, 0, rtspserver, (void*)(&port));

	bufpool_init(0,2);
	getchar();
	getchar();
	bufpool_exit();

    return 0;
}

void *rtspserver(void* p)
{
    RtspServer();
}
