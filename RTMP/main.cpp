#include <stdio.h>
#include <stdio.h>
#include <unistd.h>
#include "librtmp_send264.h"
using namespace std;

//extern NaluUnit sps;
//extern NaluUnit pps;

int main()
{
	vector<NaluUnit> input;
	parse_h264(input);
	RTMP264_Connect("rtmp://192.168.199.151:1935/hls");
	
	printf("=======RTMP=======\n");

	unsigned int nTimeStamp = 0;
	int bIsKeyFrame = 0;

	int replay = 100;
	while(replay--)
	{
		for(int i=0; i<input.size(); i++)
		{
			if(input[i].type == 5)  bIsKeyFrame = 1;
			if(input[i].type == 1)  bIsKeyFrame = 0;
			//printf("%d\n", input[i].type);

			SendH264Packet(input[i].data, input[i].size, bIsKeyFrame, nTimeStamp);
			nTimeStamp += 40;
			usleep(40*1000);
		}
	}

	printf("==================\n");

	RTMP264_Close();
	free_h264(input);
}