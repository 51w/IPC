#include <stdio.h>    
#include <stdlib.h>    
#include <string.h>
#include <unistd.h> 
#include <net/if.h>  
#include <netinet/in.h>
#include <arpa/inet.h> 
#include "bufpool2.h"

typedef struct _NaluUnit
{
	unsigned int size;
	int   type;
	char *data;
}NaluUnit;

typedef struct     
{    
    /* byte 0 */    
    unsigned char csrc_len:4; /* CC expect 0 */    
    unsigned char extension:1;/* X  expect 1, see RTP_OP below */    
    unsigned char padding:1;  /* P  expect 0 */    
    unsigned char version:2;  /* V  expect 2 */    
    /* byte 1 */    
    unsigned char payload:7; /* PT  RTP_PAYLOAD_RTSP */    
    unsigned char marker:1;  /* M   expect 1 */    
    /* byte 2,3 */    
    unsigned short seq_no;   /*sequence number*/    
    /* byte 4-7 */    
    unsigned int timestamp;    
    /* byte 8-11 */    
    unsigned int ssrc; /* stream number is used here. */    
} RTP_FIXED_HEADER;/*12 bytes*/

#define RTP_HEADER_SIZE   	   12
#define MAX_RTP_PAYLOAD_SIZE   1400//1460  1500-20-12-8
#define RTP_VERSION			   2
#define RTP_TCP_HEAD_SIZE	   4

int zzz_main()
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	int sockopt = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&sockopt, sizeof(sockopt));

    struct sockaddr_in addr = {0};		  
    addr.sin_family = AF_INET;	  
    addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    addr.sin_port = htons(5151);  

    if(bind(sockfd, (struct sockaddr*)&addr, sizeof addr) == -1)
    {
        close(sockfd);
        return false;
    }
    int size = 50*1024;
    setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (char *)&size, sizeof(size));


    struct sockaddr_in peerRtpAddr = {0};		  
    peerRtpAddr.sin_family = AF_INET;	  
	peerRtpAddr.sin_addr.s_addr = inet_addr("148.70.17.190");
    peerRtpAddr.sin_port = htons(5151);

    //
	int rtp_num = 0;
    unsigned short seq_num = 1;
    unsigned int timestamp_increse=0,ts_current=0;
    timestamp_increse=(unsigned int)(90000.0 / 25);

    int id =0;
	unsigned char stream[500*1024];
	BufData bufdata;
	bufdata.bptr = stream;
	bufdata.len  = 0;
    while(1)
    {
	if(PullBuf(&bufdata) && bufdata.len>0)
	{
        char *frameBuf  = (char*)bufdata.bptr;
        uint32_t frameSize = bufdata.len;

        ts_current += 3600;
        //cout << rtp_num++ << "   " << frameSize << endl;

        if(frameSize <= MAX_RTP_PAYLOAD_SIZE)
        {
            char sendbuf[1500];
            memset(sendbuf, 0, 1500);
            RTP_FIXED_HEADER *rtp_hdr; 
            rtp_hdr =(RTP_FIXED_HEADER*)&sendbuf[0]; 
            rtp_hdr->version = 2;   //版本号，此版本固定为2        
            rtp_hdr->payload = 96;//负载类型号，    
            rtp_hdr->ssrc    = htonl(10);//随机指定为10，并且在本RTP会话中全局唯一 
            rtp_hdr->marker  = 1;
            rtp_hdr->seq_no  = htons(seq_num++);   
            rtp_hdr->timestamp=htonl(ts_current);    
    
            char *nalu_payload=&sendbuf[12];
            memcpy(nalu_payload, frameBuf, frameSize);    
    
            int bytes = frameSize + 12;
			sendbuf[0] = 3;
			sendbuf[1] = 3;
            sendto(sockfd, sendbuf, bytes, 0, (struct sockaddr*)&peerRtpAddr, sizeof(struct sockaddr));  
            //Sleep(100);    
        }
        else
        {
            char FU_A[2] = {0};

            // 分包参考live555
            FU_A[0] = (frameBuf[0] & 0xE0) | 28;
            FU_A[1] = 0x80 | (frameBuf[0] & 0x1f);

            frameBuf  += 1;
            frameSize -= 1;

            while(frameSize + 2 > MAX_RTP_PAYLOAD_SIZE)
            {
                char sendbuf[1500];
                RTP_FIXED_HEADER *rtp_hdr; 
                rtp_hdr =(RTP_FIXED_HEADER*)&sendbuf[0]; 
                rtp_hdr->version = 2;   //版本号，此版本固定为2        
                rtp_hdr->payload = 96;//负载类型号，    
                rtp_hdr->ssrc    = htonl(10);//随机指定为10，并且在本RTP会话中全局唯一 
            
                rtp_hdr->seq_no = htons(seq_num++);
                rtp_hdr->marker = 0;
                rtp_hdr->timestamp=htonl(ts_current);

                sendbuf[RTP_HEADER_SIZE]   = FU_A[0];
                sendbuf[RTP_HEADER_SIZE+1] = FU_A[1];
                memcpy(sendbuf+RTP_HEADER_SIZE+2, frameBuf, MAX_RTP_PAYLOAD_SIZE-2);

				sendbuf[0] = 3;
				sendbuf[1] = 3;
                sendto(sockfd, sendbuf, RTP_HEADER_SIZE+MAX_RTP_PAYLOAD_SIZE, 0, (struct sockaddr*)&peerRtpAddr, sizeof(struct sockaddr));
                //sendFrame(frame.type, rtpPkt, 4+RTP_HEADER_SIZE+MAX_RTP_PAYLOAD_SIZE, 0, frame.timestamp);

                frameBuf  += MAX_RTP_PAYLOAD_SIZE - 2;
                frameSize -= MAX_RTP_PAYLOAD_SIZE - 2;

                FU_A[1] &= ~0x80;
            }

            {
                char sendbuf[1500];
                RTP_FIXED_HEADER *rtp_hdr; 
                rtp_hdr =(RTP_FIXED_HEADER*)&sendbuf[0]; 
                rtp_hdr->version = 2;   //版本号，此版本固定为2       
                rtp_hdr->payload = 96;//负载类型号，    
                rtp_hdr->ssrc    = htonl(10);//随机指定为10，并且在本RTP会话中全局唯一 

                rtp_hdr->timestamp=htonl(ts_current);
                rtp_hdr->seq_no = htons(seq_num++);
                rtp_hdr->marker = 1;

                FU_A[1] |= 0x40;
                sendbuf[RTP_HEADER_SIZE]   = FU_A[0];
                sendbuf[RTP_HEADER_SIZE+1] = FU_A[1];
                memcpy(sendbuf+RTP_HEADER_SIZE+2, frameBuf, frameSize);

                //sendFrame(frame.type, rtpPkt, 4+RTP_HEADER_SIZE+2+frameSize, 1, frame.timestamp);
				sendbuf[0] = 3;
				sendbuf[1] = 3;
                sendto(sockfd, sendbuf, RTP_HEADER_SIZE+2+frameSize, 0, (struct sockaddr*)&peerRtpAddr, sizeof(struct sockaddr));
            }
        }

	}
    }
    
    close(sockfd);
}