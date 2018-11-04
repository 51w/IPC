#include "H264VideoServerMediaSubsession.hh"
#include "VideoInput.hh"

#include <stdio.h>
#include <BasicUsageEnvironment.hh>
#include <RTSPServer.hh>

portNumBits rtspServerPortNum = 8554;
char* streamDescription = strDup("RTSP/RTP stream from Ingenic Media");

TaskScheduler* scheduler;
UsageEnvironment* env;
RTSPServer* rtspServer;

void RtspServer00()
{
	scheduler = BasicTaskScheduler::createNew();
	env = BasicUsageEnvironment::createNew(*scheduler);

	VideoInput* videoInput = VideoInput::createNew(*env, 1);
	if (videoInput == NULL) {
		printf("Video Input init failed %s: %d\n", __func__, __LINE__);
		exit(1);
	}
	
	// Create the RTSP server:
	
	// Normal case: Streaming from a built-in RTSP server:
	rtspServer = RTSPServer::createNew(*env, rtspServerPortNum, NULL);
	if (rtspServer == NULL) {
		printf("Failed to create RTSP server %s: %d\n", __func__, __LINE__);
		exit(1);
	}

	ServerMediaSession* sms_main =
		ServerMediaSession::createNew(*env, "main", NULL, streamDescription, False);

	sms_main->addSubsession(H264VideoServerMediaSubsession::createNew(sms_main->envir(), *videoInput, 1920 * 1080 * 3 / 2 + 128));

	rtspServer->addServerMediaSession(sms_main);
	char *url = rtspServer->rtspURL(sms_main);
	printf("Play this video stream using the URL: %s\n", url);
	delete[] url;

	// Begin the LIVE555 event loop:
	env->taskScheduler().doEventLoop(); // does not return
}

void RtspServerExit()
{
	delete scheduler;
	//delete env;
	//
	printf("RtspServer Exit.\n");
}