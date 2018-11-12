#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>

#include "dec264.h"
using namespace std;

typedef struct _NaluUnit
{
	unsigned int size;
	int   type;
	unsigned char *data;
}NaluUnit;
int parse_h264(std::vector<NaluUnit> &input);

int  len;
unsigned char buff[500*1024];


int main(int argc, char* argv[])
{
	vector<NaluUnit> input;
	parse_h264(input);
	cout << input.size() << endl;
	
	// start decode
	InitDecoder(AV_CODEC_ID_H264);
	buff[0] = 0x00;
	buff[1] = 0x00;
	buff[2] = 0x00;
	buff[3] = 0x01;

	YuvData Pyuv;
	unsigned char *dyuv = (unsigned char*)malloc(1920 * 540 * 3);
	
	cv::namedWindow("RTP", CV_WINDOW_NORMAL);
	cv::Mat yuvImg(540*3, 1920, CV_8UC1);


	unsigned char sps[50];
	int sps_len = 0;
	unsigned char pps[50];
	int pps_len = 0;

	sps[0] = 0x00;
	sps[1] = 0x00;
	sps[2] = 0x00;
	sps[3] = 0x01;

	pps[0] = 0x00;
	pps[1] = 0x00;
	pps[2] = 0x00;
	pps[3] = 0x01;

	for(int i = 0; i < input.size(); i++)
	{
		if (input[i].type == 7)
		{
			memcpy(sps + 4, input[i].data, input[i].size);
			sps_len = input[i].size + 4;
			continue;
		}
		if (input[i].type == 8)
		{
			memcpy(pps + 4, input[i].data, input[i].size);
			pps_len = input[i].size + 4;
			continue;
		}

		if (input[i].type == 5)
		{
			len = 0;
			memcpy(buff,           sps, sps_len);
			memcpy(buff + sps_len, pps, pps_len);
			len += sps_len + pps_len;

			buff[len + 0] = 0x00;
			buff[len + 1] = 0x00;
			buff[len + 2] = 0x00;
			buff[len + 3] = 0x01;

			memcpy(buff + 4 + len, input[i].data, input[i].size);
			len += input[i].size + 4;
		}
		else
		{
			memcpy(buff + 4, input[i].data, input[i].size);
			len = input[i].size + 4;
		}
		

		int ret = DecodeFrame(buff, len, &Pyuv);
		if (ret == 0)
		{
			//printf("========== %d %d    %d\n", Pyuv.w, Pyuv.h, Pyuv.data[1920*540*3-1]);
			memcpy(dyuv		          , Pyuv.data[0], 1920 * 1080    );
			memcpy(dyuv + 1920 * 1080 , Pyuv.data[1], 1920 * 1080 / 4);
			memcpy(dyuv + 1920 * 1350 , Pyuv.data[2], 1920 * 1080 / 4);
			yuvImg.data = dyuv;

			cv::Mat rgbImg;
			cv::cvtColor(yuvImg, rgbImg, CV_YUV2BGR_I420);

			cv::imshow("RTP", rgbImg);
			cv::waitKey(30);
		}
	}
	UninitDecoder();

	return 0;
}


int parse_h264(std::vector<NaluUnit> &input)
{
	FILE *fp = fopen("22.h264", "rb");
	fseek(fp, 0L, SEEK_END);
	int length = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	char *fbuff = (char *)malloc(length);
	fread(fbuff, 1, length, fp);


	int Spos = 0;
	int Epos = 0;
	while (Spos < length)
	{
		if (fbuff[Spos++] == 0x00 && fbuff[Spos++] == 0x00)
		{
			if (fbuff[Spos++] == 0x01)
				goto gotnal_head;
			else
			{
				Spos--;
				if (fbuff[Spos++] == 0x00 && fbuff[Spos++] == 0x01)
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
		while (Epos < length)
		{
			if (fbuff[Epos++] == 0x00 && fbuff[Epos++] == 0x00)
			{
				if (fbuff[Epos++] == 0x01)
				{
					NALdata.size = (Epos - 3) - Spos;
					break;
				}
				else
				{
					Epos--;
					if (fbuff[Epos++] == 0x00 && fbuff[Epos++] == 0x01)
					{
						NALdata.size = (Epos - 4) - Spos;
						break;
					}
				}
			}
		}
		if (Epos >= length)
		{
			NALdata.size = Epos - Spos;
			NALdata.type = fbuff[Spos] & 0x1f;
			NALdata.data = (unsigned char*)malloc(NALdata.size);
			memcpy(NALdata.data, fbuff + Spos, NALdata.size);
			input.push_back(NALdata);

			break;
		}

		NALdata.type = fbuff[Spos] & 0x1f;
		NALdata.data = (unsigned char*)malloc(NALdata.size);
		memcpy(NALdata.data, fbuff + Spos, NALdata.size);
		if (NALdata.type != 6)	input.push_back(NALdata);

		Spos = Epos - 4;
	}

	free(fbuff);
	fclose(fp);
	return 0;
}
