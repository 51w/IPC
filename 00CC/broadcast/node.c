#include <stdio.h>    
#include <stdlib.h>    
#include <string.h>

#include <unistd.h>
#include <errno.h>
#include <time.h>
//#include <signal.h>
#include <netdb.h> 
#include <net/if.h>  
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>  
#include <sys/types.h> 

#define SPORT_DEFAULT 6767
#define PKTBUF_SIZE   2048

int  UDP_found = 0;
char UDP_IP[20];
int  UDP_Port;

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
  
  int opt = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, (char *)&opt, sizeof(opt)); 

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
  if(udp_buf[0]==6&&udp_buf[1]==6)
  {
	//printf("==> [%d.%d.%d.%d]\n", udp_buf[2],udp_buf[3],udp_buf[4],udp_buf[5]);
    
    inet_ntop(AF_INET, &sender_sock->sin_addr, UDP_IP, 20);
    UDP_Port = ntohs(sender_sock->sin_port);
    printf("port %d  ip %s\n", UDP_Port, UDP_IP);
	UDP_found = 1;
  }
}

int broadcast_udp(int sockfd)
{
  struct sockaddr_in broadaddr = {0};		  
  broadaddr.sin_family = AF_INET;	  
  broadaddr.sin_addr.s_addr=htonl(INADDR_BROADCAST);
  broadaddr.sin_port = htons(SPORT_DEFAULT);
  
  uint8_t sendbuf[2];
  sendbuf[0] = 5;
  sendbuf[1] = 5;
	  
  sendto(sockfd, sendbuf, 2, 0, (struct sockaddr*)&broadaddr, sizeof(struct sockaddr)); 
}

int keep_running=1;
int run_loop(int BasicSocket)
{
  UDP_found = 0;
  uint8_t pktbuf[PKTBUF_SIZE]; 
  int rc = -1;

  broadcast_udp(BasicSocket);
  
  keep_running=2;
  while(keep_running--) 
  {  
  rc = SelectRecv(BasicSocket, 300);
  switch(rc)
  {
    case -1: 
      printf("select error!\n");
	  UDP_found = 0;
      break;
    case 0: 
      printf("Timeout!\n");
	  UDP_found = 0;
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
		UDP_found = 0;
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
  
  return UDP_found;
}

int Search_Device(char *ip, int *port)
{
  int BasicSock = -1;
	
  BasicSock = open_socket(SPORT_DEFAULT, 1 /*bind ANY*/);
  if( -1 == BasicSock ){
    printf("Failed to open main socket. %s\n", strerror(errno) );
    exit(-2);
  }else{
    printf("supernode is listening on UDP %u (main)\n", SPORT_DEFAULT);
  }
  
  if(run_loop(BasicSock))
  {
    *port = UDP_Port;
	memcpy(ip, UDP_IP, 16);
	
    return 1;
  }else
  {
    return 0;
  }
}

int main()
{
  char ip[16];
  int port = 0;
  
  int num = 6;
  while(num--)
  {
    if(Search_Device(ip, &port))
    {
      printf("====%s %d\n", ip, port);
	  break;
    }
  }
  
  return 0;
}