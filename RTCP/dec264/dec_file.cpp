#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <opencv2/opencv.hpp>
#include "dec264.h"

#define INBUF_SIZE 4096

int main()
{
	FILE *f;
	f = fopen("zz.h264", "rb");
    if (!f) {
        fprintf(stderr, "Could not open zz.h264\n");
        exit(1);
    }
	
	uint8_t inbuf[INBUF_SIZE + 64];
	uint8_t *data;
    size_t   data_size;
	
	unsigned char *dyuv = (unsigned char*)malloc(1920 * 540 * 3);
	YuvData Pyuv;
	InitDecoder(AV_CODEC_ID_H264);
	
	cv::namedWindow("RTP", CV_WINDOW_NORMAL);
	cv::Mat yuvImg(540*3, 1920, CV_8UC1);
	
	while (!feof(f)) 
	{
        data_size = fread(inbuf, 1, INBUF_SIZE, f);
        if (!data_size)
            break;

        data = inbuf;
		int ret = DecodeFrame(data, data_size, &Pyuv);
		if (ret == 0)
		{
			memcpy(dyuv		          , Pyuv.data[0], 1920 * 1080    );
			memcpy(dyuv + 1920 * 1080 , Pyuv.data[1], 1920 * 1080 / 4);
			memcpy(dyuv + 1920 * 1350 , Pyuv.data[2], 1920 * 1080 / 4);
			yuvImg.data = dyuv;

			cv::Mat rgbImg;
			cv::cvtColor(yuvImg, rgbImg, CV_YUV2BGR_I420);

			cv::imshow("RTP", rgbImg);
			cv::waitKey(25);
		}
    }
	UninitDecoder();

    fclose(f);
	return 0;
}
