#include "H264VideoServerMediaSubsession.hh"

#include <stdio.h>
#include <pthread.h>
#include <BasicUsageEnvironment.hh>
#include <RTSPServer.hh>
#include "librtspserver.hh"

portNumBits rtspServerPortNum = 8554;
char* streamDescription = strDup("RTSP/RTP stream from Ingenic Media");

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

char eventLoopWatchVariable = 0;
int isShutDown = 0;

void RtspServer00()
{
	TaskScheduler* scheduler = BasicTaskScheduler::createNew();
	UsageEnvironment* env = BasicUsageEnvironment::createNew(*scheduler);
	
	// Create the RTSP server:
	RTSPServer* rtspServer = NULL;
	// Normal case: Streaming from a built-in RTSP server:
	rtspServer = RTSPServer::createNew(*env, rtspServerPortNum, NULL);
	if (rtspServer == NULL) {
		printf("Failed to create RTSP server %s: %d\n", __func__, __LINE__);
		exit(1);
	}

	ServerMediaSession* sms_main =
		ServerMediaSession::createNew(*env, "main", NULL, streamDescription, False);

	sms_main->addSubsession(H264VideoServerMediaSubsession::createNew(sms_main->envir(), 1920*1080*3/2 + 128));

	rtspServer->addServerMediaSession(sms_main);
	char *url = rtspServer->rtspURL(sms_main);
	printf("Play this video stream using the URL: %s\n", url);
	delete[] url;

	// Begin the LIVE555 event loop:
	env->taskScheduler().doEventLoop(&eventLoopWatchVariable); // does not return
	//printf("=====%c\n", eventLoopWatchVariable);
	if (eventLoopWatchVariable != 0) {
		eventLoopWatchVariable = 0;
		isShutDown = 0;
	}
}

void *rtspserver11(void* p)
{
    RtspServer00();
}

pthread_t gs_RtspServerPid;
void RtspServer()
{
	int port = 8554;
    pthread_create(&gs_RtspServerPid, 0, rtspserver11, (void*)(&port));
}

void RtspServerExit()
{
	isShutDown = 1;
	eventLoopWatchVariable = '1';
	while (isShutDown) {
		usleep(1000);
	}
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
