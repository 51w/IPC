#include <iostream>
#include <vector>
#include <stdio.h>    
#include <stdlib.h>    
#include <string.h>
#include "RtpConnect.h"

typedef struct _NaluUnit
{
	unsigned int size;
	int   type;
	char *data;
}NaluUnit;

int parse_h264(std::vector<NaluUnit> &input);
void free_h264(std::vector<NaluUnit> &input);

int main()
{
    vector<NaluUnit> input;
    parse_h264(input);

    RtpConnect rtpconnect;
    rtpconnect.SetpeerRtpAdd("192.168.0.4");

    int npos = 0;
    while(1)
    {
        AVFrame videoFrame = {0};
        videoFrame.size = input[npos].size;
        videoFrame.buffer.reset(new char[videoFrame.size]);
        memcpy(videoFrame.buffer.get(), input[npos].data, videoFrame.size);

        rtpconnect.pushFrame(videoFrame);

        npos++;
        if(npos == input.size()) npos = 0;

        usleep(40*1000);
    }


    free_h264(input);
}


void free_h264(std::vector<NaluUnit> &input)
{
    for(int i=0; i<input.size(); i++)
	free(input[i].data);
}

int parse_h264(std::vector<NaluUnit> &input)
{
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
		if(NALdata.type != 6)	input.push_back(NALdata);

		Spos = Epos - 4;
	}

	free(fbuff);
	fclose(fp);
	return 0;
}
//