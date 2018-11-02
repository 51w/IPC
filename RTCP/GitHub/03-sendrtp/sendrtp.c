#include <stdio.h>    
#include <stdlib.h>    
#include <string.h>    
#include <unistd.h>

#include <sys/ioctl.h>
#include <net/if.h>  
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <pthread.h> 
#include "sendrtp.h"

#define  UDP_MAX_SIZE 1400
typedef struct{    
    // byte 0    
    unsigned char cc:4; 	// CC    
    unsigned char x:1;		// X    
    unsigned char p:1;  	// P
    unsigned char ver:2;	// V  
    // byte 1   
    unsigned char pt:7; 	// PT   
    unsigned char marker:1;	// M
	// byte 2,3
    unsigned short seq_no;
	// byte 4-7
    unsigned long ts;  	
	// byte 8-11  
    unsigned long ssrc; 		
} RTP_HEADER;//12 bytes
RTP_HEADER *rtp_hdr;

struct sockaddr_in serveraddr;
int sockfd; 
char sendbuf[1500];
unsigned short seq_num = 0;

int send_rtp(char *data, int size, int nal_type, int nTime)
{
	int  bytes = 0;
	char* nalu_payload;
	memset(sendbuf,0,1500);
	
	rtp_hdr = (RTP_HEADER*)&sendbuf[0];           
	rtp_hdr->cc 	= 0;
	rtp_hdr->x  	= 0;
	rtp_hdr->p  	= 0;    
	rtp_hdr->ver 	= 2;   
	rtp_hdr->pt 	= 96; 
	rtp_hdr->ssrc 	= htonl(10);
	rtp_hdr->ts 	= htonl(nTime); 
	
	if(size <= UDP_MAX_SIZE){
		//if(nal_type == 0x07||nal_type == 0x08)
		//	rtp_hdr->marker = 0;
		//else
			rtp_hdr->marker = 1;
		rtp_hdr->seq_no = htons(seq_num++);
		
		nalu_payload = &sendbuf[12];
		memcpy(nalu_payload, data, size);
      
		bytes = size + 12;
		sendto(sockfd, sendbuf, bytes, 0, (struct sockaddr*)&serveraddr, sizeof(struct sockaddr)); 
	}
	else{ //拆包发送
	//拆包个数
		int packetNum = size/UDP_MAX_SIZE;    
		if(size%UDP_MAX_SIZE != 0)    
			packetNum ++;    
		int lastPackSize = size - (packetNum-1)*UDP_MAX_SIZE;    
		int packetIndex  = 1;
		
		rtp_hdr->seq_no = htons(seq_num++);
		rtp_hdr->marker = 0;

		sendbuf[12] = 0x1c | (data[0] & ~0x1F);		
		//发送第一个的FU，S=1，E=0，R=0 
		sendbuf[13] = (1<<7) | nal_type;
	
		nalu_payload = &sendbuf[14];   
		memcpy(nalu_payload, data+1, UDP_MAX_SIZE-1);   
		bytes = UDP_MAX_SIZE+13;//除去起始前缀和NALU头）加上rtp_header，fu_ind，fu_hdr的固定长度14字节     
		sendto(sockfd, sendbuf, bytes, 0, (struct sockaddr*)&serveraddr, sizeof(struct sockaddr));  

		for(packetIndex=2; packetIndex<packetNum; packetIndex++)    
        {    
			rtp_hdr->seq_no = htons(seq_num++); //序列号，每发送一个RTP包增1    
			rtp_hdr->marker = 0;    
  
			sendbuf[12] = 0x1c | (data[0] & ~0x1F);  

			//发送第一个的FU，S=0，E=0，R=0 
			sendbuf[13] = (0<<7) | nal_type;   

			nalu_payload = &sendbuf[14]; 
			memcpy(nalu_payload, data+(packetIndex-1)*UDP_MAX_SIZE, UDP_MAX_SIZE);
			bytes = UDP_MAX_SIZE+14;
			sendto(sockfd, sendbuf, bytes, 0, (struct sockaddr*)&serveraddr, sizeof(struct sockaddr));                
		}
		
		//发送最后一个的FU，S=0，E=1，R=0    
		rtp_hdr->seq_no = htons(seq_num++);    
		rtp_hdr->marker = 1;    
 
		sendbuf[12] = 0x1c | (data[0] & ~0x1F);    
		//发送第一个的FU，S=0，E=0，R=0 
		sendbuf[13] = (1<<6) | nal_type;   

		nalu_payload = &sendbuf[14];  
		memcpy(nalu_payload, data+(packetIndex-1)*UDP_MAX_SIZE, lastPackSize);
		bytes = lastPackSize+14;    
		sendto(sockfd, sendbuf, bytes, 0, (struct sockaddr*)&serveraddr, sizeof(struct sockaddr));		
	}
}

int init_rtp(char* ip, int port)
{
	int result;
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);  
    if(sockfd == -1) {
		printf("socket error");		
		return -1;  
    }
	
	serveraddr.sin_family 		= AF_INET;  
    serveraddr.sin_port 		= htons(port);  
    serveraddr.sin_addr.s_addr 	= inet_addr(ip);;  
    bzero(&(serveraddr.sin_zero), 8); 
	
	int on = 1;
	result = ioctl(sockfd, FIONBIO, &on);
	if(result == -1 ){
		close(sockfd);
		return -1;
	}
	else{
		int reuse = 1;
		result = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,(const char*)&reuse, sizeof(reuse));
		return result;
	}
	//printf("rtp init successed!\n");	
	return 0;
}

int exit_rtp()
{
	close(sockfd);
}