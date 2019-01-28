#include <iostream>
#include <vector>
#include <chrono>
#include <stdio.h>    
#include <stdlib.h>    
#include <string.h>
#include <unistd.h> 
#include <net/if.h>  
#include <netinet/in.h>
#include <arpa/inet.h> 
using namespace std;

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
#define MAX_RTP_PAYLOAD_SIZE   1460//1460  1500-20-12-8
#define RTP_VERSION			   2
#define RTP_TCP_HEAD_SIZE	   4

int parse_h264(std::vector<NaluUnit> &input);

uint32_t getTimeStamp()
{
    auto timePoint = chrono::time_point_cast<chrono::microseconds>(chrono::high_resolution_clock::now());
    return (uint32_t)((timePoint.time_since_epoch().count() + 500) / 1000 * 90 );
}

int main()
{
    vector<NaluUnit> input;
    parse_h264(input);
    cout << input.size() << endl;

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in addr = {0};		  
    addr.sin_family = AF_INET;	  
    addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    addr.sin_port = htons(45678);  

    if(bind(sockfd, (struct sockaddr*)&addr, sizeof addr) == -1)
    {
        cout << "bind error" << endl;
        close(sockfd);
        return false;
    }
    int size = 50*1024;
    setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (char *)&size, sizeof(size));


    struct sockaddr_in peerRtpAddr = {0};		  
    peerRtpAddr.sin_family = AF_INET;	  
    //peerRtpAddr.sin_addr.s_addr = inet_addr("192.168.0.4");
	//peerRtpAddr.sin_port = htons(45678);
	peerRtpAddr.sin_addr.s_addr = inet_addr("0.0.0.0");
    peerRtpAddr.sin_port = htons(56789);

    //
	int rtp_num = 0;
    unsigned short seq_num = 1;
    unsigned int timestamp_increse=0,ts_current=0;
    timestamp_increse=(unsigned int)(90000.0 / 25);

    int id =0;
    while(1)
    //for(int id=0; id<input.size(); id++)
    {
        char *frameBuf  = input[id].data;
        uint32_t frameSize = input[id].size;

        ts_current = getTimeStamp();
        //ts_current += 3600;
        cout << rtp_num++ << "   " << frameSize << endl;

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
                sendto(sockfd, sendbuf, RTP_HEADER_SIZE+2+frameSize, 0, (struct sockaddr*)&peerRtpAddr, sizeof(struct sockaddr));
            }

            /*
            int packetNum = input[id].size / UDP_MAX_SIZE;    
            if(input[id].size % UDP_MAX_SIZE != 0)    
                packetNum ++;

            int lastPackSize = input[id].size - (packetNum-1)*UDP_MAX_SIZE;    
            int packetIndex = 1;

            ts_current=ts_current+timestamp_increse;
            rtp_hdr->timestamp=htonl(ts_current);

            rtp_hdr->seq_no = htons(seq_num++);
            rtp_hdr->marker = 0;

            sendbuf[12] = 0x1c | (input[id].data[0] & ~0x1F);
			sendbuf[13] = (1<<7) | input[id].type;

            char *nalu_payload = &sendbuf[14];//同理将sendbuf[14]赋给nalu_payload    
            memcpy(nalu_payload, input[id].data+1, UDP_MAX_SIZE);//去掉NALU头    
            int bytes = UDP_MAX_SIZE+14;
            sendto(sockfd, sendbuf, bytes, 0, (struct sockaddr*)&peerRtpAddr, sizeof(struct sockaddr));

            for(packetIndex=2; packetIndex<packetNum; packetIndex++)    
            {    
                rtp_hdr->seq_no = htons(seq_num++);   
                rtp_hdr->marker=0;
				sendbuf[12] = 0x1c | (input[id].data[0] & ~0x1F);
				sendbuf[13] = (0<<7) | input[id].type;			
    
                nalu_payload = &sendbuf[14];//同理将sendbuf[14]的地址赋给nalu_payload    
                memcpy(nalu_payload, input[id].data + (packetIndex-1)*UDP_MAX_SIZE+1, UDP_MAX_SIZE);//去掉起始前缀的nalu剩余内容写入sendbuf[14]开始的字符串。    
                bytes=UDP_MAX_SIZE+14;//获得sendbuf的长度,为nalu的长度（除去原NALU头）加上rtp_header，fu_ind，fu_hdr的固定长度14字节    
                //send( socket1, sendbuf, bytes, 0 );//发送rtp包   
                sendto(sockfd, sendbuf, bytes, 0, (struct sockaddr*)&peerRtpAddr, sizeof(struct sockaddr));                
            }

            rtp_hdr->seq_no = htons(seq_num++);        
            rtp_hdr->marker=1;
            sendbuf[12] = 0x1c | (input[id].data[0] & ~0x1F);
			sendbuf[13] = (1<<6) | input[id].type;

            nalu_payload = &sendbuf[14];//同理将sendbuf[14]的地址赋给nalu_payload    
            memcpy(nalu_payload, input[id].data+(packetIndex-1)*UDP_MAX_SIZE+1,lastPackSize-1);//将nalu最后剩余的-1(去掉了一个字节的NALU头)字节内容写入sendbuf[14]开始的字符串。    
            bytes=lastPackSize-1+14;
            sendto(sockfd, sendbuf, bytes, 0, (struct sockaddr*)&peerRtpAddr, sizeof(struct sockaddr));
            */
        }
        usleep(50*1000);

        id++;
        if(id == input.size()) id =0;
    }
    
    close(sockfd);
}

int parse_h264(std::vector<NaluUnit> &input)
{
	FILE *fp = fopen("../../h264/11.h264", "rb");
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