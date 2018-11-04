#include <stdio.h>    
#include <stdlib.h>    
#include <string.h>
#include <unistd.h> 
#include <net/if.h>  
#include <netinet/in.h>
#include <arpa/inet.h> 

#define RTP_HEADER_SIZE   	   12
#define MAX_RTP_PAYLOAD_SIZE   7000//1460  1500-20-12-8
#define RTP_VERSION			   2
#define RTP_TCP_HEAD_SIZE	   4

int main()
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in addr = {0};		  
    addr.sin_family = AF_INET;	  
    addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    addr.sin_port = htons(45678);  

    if(bind(sockfd, (struct sockaddr*)&addr, sizeof addr) == -1)
    {
        close(sockfd);
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
    unsigned short seq_num = 1;
    unsigned int timestamp_increse=0,ts_current=0;
    timestamp_increse=(unsigned int)(90000.0 / 25);

    char sendbuf[MAX_RTP_PAYLOAD_SIZE];
	int i=0;
	for(i=0; i<MAX_RTP_PAYLOAD_SIZE; i++)
		sendbuf[i] = i/7;
	
    while(1)
    {
        getchar();getchar();
        
		sendto(sockfd, sendbuf, MAX_RTP_PAYLOAD_SIZE, 0, 
		(struct sockaddr*)&peerRtpAddr, sizeof(struct sockaddr));  
    }
    
    close(sockfd);
}