#pragma once
#include <vector>

typedef struct _NaluUnit
{
	unsigned int size;
	int   type;
	char *data;
}NaluUnit;


//librtmp
int RTMP264_Connect(const char* url);    

int RTMP264_Close();

int SendH264Packet(char *data, unsigned int size, int bIsKeyFrame, unsigned int nTimeStamp);


//file parse
int free_h264(std::vector<NaluUnit> &input);

int parse_h264(std::vector<NaluUnit> &input);