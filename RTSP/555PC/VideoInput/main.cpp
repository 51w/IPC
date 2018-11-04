#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

pthread_t gs_VencPid;
void *rtspserver(void* p);
extern void RtspServer00();
extern void RtspServerExit();

int main()
{
    int port = 8554;
    pthread_create(&gs_VencPid, 0, rtspserver, (void*)(&port));

	getchar();
	getchar();
	RtspServerExit();

    return 0;
}

void *rtspserver(void* p)
{
    RtspServer00();
}
