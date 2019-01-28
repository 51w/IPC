#include <stdio.h>    
#include <stdlib.h>    
#include <string.h>

#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <netdb.h> 
#include <net/if.h>  
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>  
#include <sys/types.h> 
#include "h264.h"
#include "bufpool2.h"

#define SDST_DEFAULT  "148.70.17.190"
#define SPORT_DEFAULT 5151
#define PKTBUF_SIZE   2048
int nalu_len = 0;
unsigned char _buff[600*1024];
int _buffsize = 0;

int open_socket(int local_port, int bind_any)
{
  int sockfd;
  struct sockaddr_in local_address;
  int sockopt = 1;

  if((sockfd = socket(PF_INET, SOCK_DGRAM, 0))  < 0) {
    printf("Unable to create socket [%s][%d]\n",
		strerror(errno), sockfd);
    return(-1);
  }
#ifndef WIN32
  /* fcntl(sockfd, F_SETFL, O_NONBLOCK); */
#endif

  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&sockopt, sizeof(sockopt));
  
  memset(&local_address, 0, sizeof(local_address));
  local_address.sin_family = AF_INET;
  local_address.sin_port = htons(local_port);
  local_address.sin_addr.s_addr = htonl(bind_any?INADDR_ANY:INADDR_LOOPBACK);
  if(bind(sockfd, (struct sockaddr*) &local_address, sizeof(local_address)) == -1) {
    printf("Bind error [%s]\n", strerror(errno));
    return(-1);
  }

  return(sockfd);
}

int SelectRecv(int RecvSock, int maxMsec)
{
  fd_set socket_mask; 
  FD_ZERO(&socket_mask);
  FD_SET(RecvSock, &socket_mask);
	
  struct timeval timeout;
  timeout.tv_sec	=  maxMsec/1000;
  timeout.tv_usec	= (maxMsec%1000)*1000;
  
  int result = -1;
  result = select(RecvSock+1,&socket_mask,NULL,NULL,&timeout);
  if(result > 0)
  {
    if(FD_ISSET(RecvSock, &socket_mask)>0) result = 1;
    else result = -1;
  }
  return result;
}

int process_udp(int sss, const struct sockaddr_in * sender_sock,
  uint8_t * udp_buf, size_t udp_size)
{

//printf("==> [%d %d %d %d=========%ld]\n", udp_buf[0],udp_buf[1],udp_buf[10]&0x1f, udp_buf[11]&0x1f, udp_size);
    
  NALU_HEADER *nalu_hd = (NALU_HEADER *)&udp_buf[10];
  
  if(nalu_hd->TYPE == 28)
  {
    FU_HEADER *fu_hd = (FU_HEADER *)&udp_buf[11];
	nalu_len = nalu_len + udp_size - 12;
	
	if(fu_hd->E == 1) 
	{
		printf("len: %6d \n",nalu_len + 1); nalu_len = 0;
	  memcpy(_buff+_buffsize, udp_buf+12, udp_size - 12);
	  _buffsize += udp_size - 12;
	  
      BufData data;
	  data.bptr = _buff;
	  data.len  = _buffsize;
      PushBuf(data);	  
	}
	else if(fu_hd->S == 1) 
	{
	  unsigned char h264_nal_header = (fu_hd->TYPE & 0x1f) | (nalu_hd->NRI << 5) | (nalu_hd->F << 7);
	  _buff[0]  = h264_nal_header;
	  _buffsize = 1;
	  
	  memcpy(_buff+_buffsize, udp_buf+12, udp_size - 12);
	  _buffsize += udp_size - 12;
	}
	else
	{
	  memcpy(_buff+_buffsize, udp_buf+12, udp_size - 12);
	  _buffsize += udp_size - 12;
	}
  }
  else 
  {
    BufData data;
	data.bptr = udp_buf +10;
	data.len  = udp_size-10;
    PushBuf(data);
  }
}

int broadcast_udp(int sockfd)
{
  struct sockaddr_in broadaddr = {0};		  
  broadaddr.sin_family = AF_INET;	  
  broadaddr.sin_addr.s_addr=inet_addr(SDST_DEFAULT);
  broadaddr.sin_port = htons(SPORT_DEFAULT);
  
  uint8_t sendbuf[2];
  sendbuf[0] = 1;
  sendbuf[1] = 1;
	  
  sendto(sockfd, sendbuf, 2, 0, (struct sockaddr*)&broadaddr, sizeof(struct sockaddr)); 
}

int keep_running=1;
int run_loop(int BasicSocket)
{
  uint8_t pktbuf[PKTBUF_SIZE]; 
  int rc = -1;
  
  while(keep_running) 
  {  
  rc = SelectRecv(BasicSocket, 6000);
  switch(rc)
  {
    case -1: 
      printf("select error!\n");
      break;
    case 0: 
      //printf("Timeout!\n");
	  broadcast_udp(BasicSocket);
      break;
    default:
    {
      struct sockaddr_in sendsock;
      socklen_t socklen = sizeof(struct sockaddr_in);
		
      int result = recvfrom(BasicSocket, pktbuf, PKTBUF_SIZE, 0, 
            (struct sockaddr *)&sendsock, (socklen_t *)&socklen);
      if(result<0)
      {
        printf("recvfrom() failed %d errno %d (%s)", result, errno, strerror(errno));
        keep_running=0;
        break;
      }
		
      if(result>0)
      {
        process_udp(BasicSocket, &sendsock, pktbuf, result);
        break;
      }
      break;
    }
  }
  
  }
	
  if(BasicSocket >= 0)
    close(BasicSocket);
  BasicSocket=-1;
  
  return 0;
}

void *client_main(void* p)
{
  int BasicSock = -1;
	
  BasicSock = open_socket(SPORT_DEFAULT, 1 /*bind ANY*/);
  if( -1 == BasicSock ){
    printf("Failed to open main socket. %s\n", strerror(errno) );
    exit(-2);
  }else{
    printf("supernode is listening on UDP %u (main)\n", SPORT_DEFAULT);
  }
  
  run_loop(BasicSock);
}

pthread_t pt_client;
int Client_UDP()
{
  pthread_create(&pt_client, 0, client_main, NULL);
  return 0;
}

void *dec_main(void* p)
{
	unsigned char stream[500*1024];
	BufData bufdata;
	bufdata.bptr = stream;
	bufdata.len  = 0;
	while(1)
	{
		if(PullBuf(&bufdata) && bufdata.len>0)
		{
			printf("************  [%d]\n", bufdata.len);
		}
	}
}

int main()
{
	pthread_t pt_dec;
	pthread_create(&pt_dec, 0, dec_main, NULL);
	
	Client_UDP();
	while(1)
	{
		printf("=======\n");
		usleep(1000*1000);
	}
}