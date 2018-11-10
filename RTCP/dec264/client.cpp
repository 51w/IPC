#include <memory.h> 
#include <net/if.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "h264.h" 
#include <arpa/inet.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include <opencv2/opencv.hpp>
#include "dec264.h"
pthread_t g_decThreadId;
pthread_mutex_t mutex;

unsigned char dbuff[2000];
int dbuff_size = 0;
unsigned char *dyuv;

int rtpnum = 0;
int nalu_len = 0;


#define BUFSIZEMAX 1024*1024
unsigned char _buff[BUFSIZEMAX];
int _buffsize = 0;

void *UDP_Thread(void *)
{
	printf("===================UDP_Thread Enter=================\n");
	YuvData Pyuv;
	int ddec;
	
	while(1)
	{
		pthread_mutex_lock(&mutex);
		if(_buffsize > 0)
		{
			ddec = DecodeFrame(_buff, _buffsize, &Pyuv);
			
			cv::Mat yuvImg(540*3, 1920, CV_8UC1);
			if(ddec == 0)
			{
				//printf("*******************\n");
				
				memcpy(dyuv		          , Pyuv.data[0], 1920 * 1080    );
				memcpy(dyuv + 1920 * 1080 , Pyuv.data[1], 1920 * 1080 / 4);
				memcpy(dyuv + 1920 * 1350 , Pyuv.data[2], 1920 * 1080 / 4);
				yuvImg.data = dyuv;

				cv::Mat rgbImg;
				cv::cvtColor(yuvImg, rgbImg, CV_YUV2BGR_I420);
				
				cv::imshow("RTP", rgbImg);
				cv::waitKey(1);
			}
			_buffsize = 0;
		}
		pthread_mutex_unlock(&mutex);
	}
	
	printf("===================UDP_Thread Exit =================\n");
}

void decode_rtp2h264(unsigned char *rtp_buf, int len)
{
	YuvData Pyuv;
	int ddec;
	//struct timeval st, et;
	//gettimeofday(&st, NULL);
	
    NALU_HEADER *nalu_header;
    FU_HEADER   *fu_header;
    unsigned char h264_nal_header;

    nalu_header = (NALU_HEADER *)&rtp_buf[12];
    if (nalu_header->TYPE == 28) { /* FU-A */
        fu_header = (FU_HEADER *)&rtp_buf[13];

        nalu_len = nalu_len + len - 14;

        if (fu_header->E == 1) 
		{
			//memcpy(dbuff, &rtp_buf[14], len - 14);
			//dbuff_size = len - 14;
			//ddec = DecodeFrame(dbuff, dbuff_size, &Pyuv);
			
			pthread_mutex_lock(&mutex);
			if(_buffsize + len - 14 < BUFSIZEMAX)
			{
				memcpy(_buff+_buffsize, &rtp_buf[14], len - 14);
				_buffsize += len - 14;
			}
			pthread_mutex_unlock(&mutex);
        } 
		else if (fu_header->S == 1) 
		{
            h264_nal_header = (fu_header->TYPE & 0x1f) 
                | (nalu_header->NRI << 5)
                | (nalu_header->F << 7);

			dbuff[0] = 0x00;
			dbuff[1] = 0x00;
			dbuff[2] = 0x00;
			dbuff[3] = 0x01;
			dbuff[4] = h264_nal_header;
			memcpy(dbuff+5, &rtp_buf[14], len - 14);
			dbuff_size = len - 9;
			//ddec = DecodeFrame(dbuff, dbuff_size, &Pyuv);
			
			pthread_mutex_lock(&mutex);
			if(_buffsize + dbuff_size < BUFSIZEMAX)
			{
				memcpy(_buff+_buffsize, dbuff, dbuff_size);
				_buffsize += dbuff_size;
			}
			pthread_mutex_unlock(&mutex);
        } 
		else 
		{
			// memcpy(dbuff, &rtp_buf[14], len - 14);
			// dbuff_size = len - 14;
			// ddec = DecodeFrame(dbuff, dbuff_size, &Pyuv);
			
			pthread_mutex_lock(&mutex);
			if(_buffsize + len - 14 < BUFSIZEMAX)
			{
				memcpy(_buff+_buffsize, &rtp_buf[14], len - 14);
				_buffsize += len - 14;
			}
			pthread_mutex_unlock(&mutex);
        }
    } 
	else 
	{
		dbuff[0] = 0x00;
		dbuff[1] = 0x00;
		dbuff[2] = 0x00;
		dbuff[3] = 0x01;
		memcpy(dbuff+4, &rtp_buf[12], len - 12);
		dbuff_size = len - 8;
		//ddec = DecodeFrame(dbuff, dbuff_size, &Pyuv);
		
		pthread_mutex_lock(&mutex);
		if(_buffsize + dbuff_size < BUFSIZEMAX)
		{
			memcpy(_buff+_buffsize, dbuff, dbuff_size);
			_buffsize += dbuff_size;
		}
		pthread_mutex_unlock(&mutex);
    }
	
	/*
	cv::Mat yuvImg(540*3, 1920, CV_8UC1);
	if(ddec == 0)
	{
		//printf("*******************\n");
		
		memcpy(dyuv		          , Pyuv.data[0], 1920 * 1080    );
		memcpy(dyuv + 1920 * 1080 , Pyuv.data[1], 1920 * 1080 / 4);
		memcpy(dyuv + 1920 * 1350 , Pyuv.data[2], 1920 * 1080 / 4);
		yuvImg.data = dyuv;

		//cv::Mat rgbImg;
		//cv::cvtColor(yuvImg, rgbImg, CV_YUV2BGR_I420);
		
		//cv::imshow("RTP", rgbImg);
		//cv::waitKey(1);
	}*/
	
	//gettimeofday(&et, NULL);
	//printf("==========%d\n", (et.tv_sec-st.tv_sec)*1000 + (et.tv_usec-st.tv_usec)/1000);


    //fflush(savefp);
    return;
}

// void show_buf(unsigned char *rtp_buf, int len)
// {
    // int temp = len > 16 ? 16 : len;
    // for(int i=0;i<temp;i++)
    // {
        // printf("%0x ",rtp_buf[i]);
    // }
    // printf("\n");
// }

int main(int argc, char **argv)
{
    int socket_s = -1;
    struct sockaddr_in si_me;
    int ret;
    unsigned char buf[1500];

    //init socket
    socket_s = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_s < 0) {
        printf("socket fail!\n");
        exit(1);
    }

    bzero(&si_me, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(56789);
    si_me.sin_addr.s_addr = inet_addr("0.0.0.0");

    if(bind(socket_s,(struct sockaddr *)&si_me,sizeof(struct sockaddr_in))<0) 
	{ 
		fprintf(stderr,"Bind Error:\n"); 
		exit(1); 
	}
	
	struct sockaddr_in fromAddr;
    int sockLen;
	fd_set fd;
	timeval timeout;
	
	
	cv::namedWindow("RTP", CV_WINDOW_NORMAL);
	InitDecoder(AV_CODEC_ID_H264);
	dyuv = (unsigned char*)malloc(1920 * 540 * 3);

    //while(1)
    //{
    //    ret = recv(socket_s, buf, sizeof(buf), 0);
    //    if (ret < 0)
	//	{
    //        fprintf(stderr, "recv fail\n");
    //        continue;
    //    }
    //    //show_buf(buf,ret);
		
    //    decode_rtp2h264(buf, ret);
    //}
	
	ret = pthread_create(&g_decThreadId, NULL, UDP_Thread, NULL);
	if( ret < 0)
	{
		printf("g_decThreadId  thread error\r\n");
		return -1;
	}
	
	while(1)
	{
		FD_ZERO(&fd);
		FD_SET(socket_s, &fd);
		
		// 设置超时时间为6s
		timeout.tv_sec = 6;	
		timeout.tv_usec = 0;
		ret = select(socket_s+1, &fd, NULL, NULL, &timeout);
		if(ret == 0)
		{
			fprintf(stderr, "timeout\n");
		}
		else if(ret < 0)
		{
			printf("SYSTEM ERROR at %s_%d\n",__FILE__,__LINE__);
			close(socket_s);
			return -1;
		}
		else
		{
			if ( FD_ISSET(socket_s, &fd) > 0 )
			{
				int buf_size = recvfrom(socket_s, buf, sizeof(buf), 0, (struct sockaddr *)&fromAddr, (socklen_t *)&sockLen );
				
				if(buf_size <= 0)
				{
					printf("recvfrom ERROR at %s_%d\n",__FILE__,__LINE__);
				}
				else
				{
					decode_rtp2h264(buf, buf_size);
				}
			}
			else
  			{
  				printf("SYSTEM ERROR at %s_%d\n",__FILE__,__LINE__);
  			}
		}
		
	}
	
	UninitDecoder();
    close(socket_s);

    return 0;
}