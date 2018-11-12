#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/ioctl.h>

#include "VideoInput.hh"
#include "H264VideoStreamSource.hh"

#define TAG 					"sample-RTSPServer"

#define CONFIG_SENSOR_NAME		"sc2135"
#define CONFIG_SENSOR_ADDR		0x30
#define CONFIG_FPS_NUM			25
#define CONFIG_FPS_DEN			1
#define CONFIG_VIDEO_WIDTH		1920
#define CONFIG_VIDEO_HEIGHT		1080
#define CONFIG_VIDEO_BITRATE 	6000

Boolean VideoInput::fpsIsOn[MAX_STREAM_CNT] = {False, False};
Boolean VideoInput::fHaveInitialized = False;
double VideoInput::gFps[MAX_STREAM_CNT] = {0.0, 0.0};
double VideoInput::gBitRate[MAX_STREAM_CNT] = {0.0, 0.0};

static inline int close_stream_file(int fd)
{
	return close(fd);
}

VideoInput* VideoInput::createNew(UsageEnvironment& env, int streamNum) 
{
    if (!fHaveInitialized) 
	{
		if (!initialize(env)) {
			printf("%s: %d\n", __func__, __LINE__);
			return NULL;
		}
		fHaveInitialized = True;
    }

	VideoInput *videoInput = new VideoInput(env, streamNum);

    return videoInput;
}

VideoInput::VideoInput(UsageEnvironment& env, int streamNum)
	: Medium(env), global_file264(0), fVideoSource(NULL), fpsIsStart(False), fontIsStart(False),
	  osdIsStart(False), osdStartCnt(0), nrFrmFps(0),
	  totalLenFps(0), startTimeFps(0), streamNum(streamNum)
{
}

VideoInput::~VideoInput() {
}


bool VideoInput::initialize(UsageEnvironment& env) 
{
	return true;
}

FramedSource* VideoInput::videoSource()
{
	fVideoSource = new H264VideoStreamSource(envir(), *this);
	return fVideoSource;
}

int VideoInput::getStream(void* to, unsigned int* len, struct timeval* timestamp, unsigned fMaxSize) 
{
	unsigned int stream_len = 0;
	printf("=============%d\n", dninput[npos].size);

	stream_len = dninput[npos].size;
	if (stream_len > fMaxSize) {
		printf("[%d]  drop stream: length=%u, fMaxSize=%d\n", streamNum, stream_len, fMaxSize);
		stream_len = 0;
		goto out;
	}

	memcpy((void*)to, (void *)(dninput[npos].data), stream_len);
	gettimeofday(timestamp, NULL);
	
	if(dninput[npos].type != 7 && dninput[npos].type != 8 && dninput[npos].type != 6)
	usleep(33*1000);

	npos++;
	if(npos == dninput.size()) npos = 0;

out:
	*len = stream_len;

	return 0;
}

int VideoInput::pollingStream(void)
{
	return 0;
}

int VideoInput::streamOn(void)
{
	if(global_file264 == 0)
	{
		global_file264 = 1;
		parse_h264(dninput);
	}

	return 0;
}

int VideoInput::streamOff(void)
{
	//for(int i=0; i<dninput.size(); i++)
	//	free(dninput[i].data);
	npos = 0;

	return 0;
}

int VideoInput::parse_h264(std::vector<NaluUnit> &input)
{
	//printf("============cuc_ieschool.264==============\n");
	FILE *fp = fopen("../../h264/22.h264", "rb");
	fseek(fp, 0L, SEEK_END);
	int length = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	char *fbuff = (char *)malloc(length);
	fread(fbuff, 1, length, fp);

	
	int Spos = 0;
	int Epos = 0;
	while(Spos < length)
	{
		if(fbuff[Spos++] == 0x00 && fbuff[Spos++] == 0x00)
		{
			if(fbuff[Spos++] == 0x01)
				goto gotnal_head;
			else
			{
				Spos--;
				if(fbuff[Spos++] == 0x00 && fbuff[Spos++] == 0x01)
					goto gotnal_head;
				else
					continue;
			}
		}
		else
		{
			continue;
		}
		
	gotnal_head:
		Epos = Spos;
		//int size = 0;
		NaluUnit NALdata;
		while(Epos < length)
		{
			if(fbuff[Epos++] == 0x00 && fbuff[Epos++] == 0x00)
			{
				if(fbuff[Epos++] == 0x01)
				{
					NALdata.size = (Epos-3)-Spos;
					break;
				}
				else
				{
					Epos--;
					if(fbuff[Epos++] == 0x00 && fbuff[Epos++] == 0x01)
					{	
						NALdata.size = (Epos-4)-Spos;
						break;
					}
				}
			}
		}
		if(Epos >= length)
		{
			NALdata.size = Epos - Spos;
			NALdata.type = fbuff[Spos]&0x1f;
			NALdata.data = (char*)malloc(NALdata.size);
			memcpy(NALdata.data, fbuff+Spos, NALdata.size);
			input.push_back(NALdata);

			break;
		}

		NALdata.type = fbuff[Spos]&0x1f;
		NALdata.data = (char*)malloc(NALdata.size);
		memcpy(NALdata.data, fbuff+Spos, NALdata.size);
		
		if(NALdata.type != 6)	
		input.push_back(NALdata);

		Spos = Epos - 4;
	}

	free(fbuff);
	fclose(fp);
	return 0;
}
