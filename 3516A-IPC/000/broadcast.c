#include <stdio.h>    
#include <stdlib.h>    
#include <string.h>

#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <netdb.h> 
#include <net/if.h>  
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>  
#include <sys/types.h> 

#define SPORT_DEFAULT 6767
#define PKTBUF_SIZE   2048

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

  //int opt = 1;
  //setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, (char *)&opt, sizeof(opt)); 

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

int SelectSend(int SendSock, int maxMsec)
{
  fd_set socket_mask; 
  FD_ZERO(&socket_mask);
  FD_SET(SendSock, &socket_mask);

  struct timeval timeout;
  timeout.tv_sec	=  maxMsec/1000;
  timeout.tv_usec	= (maxMsec%1000)*1000;
	
  int result = -1;
  result = select(SendSock+1,NULL,&socket_mask,NULL,&timeout);
  if(result > 0)
  {
    if(FD_ISSET(SendSock, &socket_mask)>0) result = 1;
    else result = -1;
  }
  return result;
}

int local_ip(const char *eth, char *ip, int size)
{  
  int sd;  
  struct sockaddr_in sin;  
  struct ifreq ifr;  

  sd = socket(AF_INET, SOCK_DGRAM, 0);  
  if (-1 == sd)  
  {  
    printf("socket error: %s\n", strerror(errno));  
    return -1;        
  }  
  strncpy(ifr.ifr_name, eth, IFNAMSIZ);  
  ifr.ifr_name[IFNAMSIZ - 1] = 0;  

  // if error: No such device  
  if (ioctl(sd, SIOCGIFADDR, &ifr) < 0)  
  {  
    printf("ioctl error: %s\n", strerror(errno));  
    close(sd);  
    return -1;  
  }  
  
  memcpy(&sin, &ifr.ifr_addr, sizeof(sin));  
  snprintf(ip, size, "%s", inet_ntoa(sin.sin_addr));  

  close(sd);  
  return 0;  
}

int local_mac(const char *eth, char *mac, int size)  
{  
  struct ifreq ifr;
  int sd;
    
  bzero(&ifr, sizeof(struct ifreq));  
  if((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)  
  {  
    printf("get %s mac address socket creat error\n", eth);  
    return -1;  
  }  
  strncpy(ifr.ifr_name, eth, sizeof(ifr.ifr_name) - 1);  

  if(ioctl(sd, SIOCGIFHWADDR, &ifr) < 0)  
  {  
    printf("get %s mac address error\n", eth);  
    close(sd);  
    return -1;  
  }  

  snprintf(mac, size, "%02x:%02x:%02x:%02x:%02x:%02x",  
    (unsigned char)ifr.ifr_hwaddr.sa_data[0],   
    (unsigned char)ifr.ifr_hwaddr.sa_data[1],  
    (unsigned char)ifr.ifr_hwaddr.sa_data[2],   
    (unsigned char)ifr.ifr_hwaddr.sa_data[3],  
    (unsigned char)ifr.ifr_hwaddr.sa_data[4],  
    (unsigned char)ifr.ifr_hwaddr.sa_data[5]);  
  
  close(sd);  
  return 0;  
}

int domain_ip(const char *domain)  
{
/*  char **pptr;
  struct hostent *hptr; 
  hptr = gethostbyname(domain);
  if(NULL == hptr)
  {
    printf("gethostbyname error for host:%s/n", domain);
    return -1;
  }
  for(pptr=hptr->h_addr_list; *pptr != NULL; pptr++)  
  {
	  if(NULL != inet_ntop(hptr->h_addrtype, *pptr, ip, size))
	  {  return 0; } //只获取第一个IP
  }
  return -1;*/   //过时
  struct sockaddr_in *sockaddr_ipv4;
  struct addrinfo *result = NULL;
  struct addrinfo *ptr = NULL;
  struct addrinfo hints = {0};
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  
  int c = getaddrinfo(domain, 0, &hints, &result);
  if(c != 0){
    printf("getaddrinfo error for host:%s/n", domain);
    return -1;
  }else{
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next){
      	switch (ptr->ai_family) {
		case AF_UNSPEC:
		  break;
		case AF_INET:
		  sockaddr_ipv4 = (struct sockaddr_in *) ptr->ai_addr;
		  printf("\tIPv4 address %s\n", inet_ntoa(sockaddr_ipv4->sin_addr));
		  break;
		case AF_INET6:
		  break;
		default:
		  printf("Other %ld\n", ptr->ai_family);
		  break;
		}
	}
  }
}

int broadcast_back(int sockfd)
{
  struct sockaddr_in broadaddr = {0};		  
  broadaddr.sin_family = AF_INET;	  
  broadaddr.sin_addr.s_addr=htonl(INADDR_BROADCAST);
  broadaddr.sin_port = htons(SPORT_DEFAULT);
  
  uint8_t sendbuf[2];
  sendbuf[0] = 6;
  sendbuf[1] = 6;
	  
  sendto(sockfd, sendbuf, 2, 0, (struct sockaddr*)&broadaddr, sizeof(struct sockaddr)); 
}

static int process_udp(int sss, const struct sockaddr_in * sender_sock,
  const uint8_t * udp_buf, size_t udp_size)
{
  //printf("==> [%d %d %d]\n", udp_buf[0],udp_buf[1],udp_buf[2]);	
  if(udp_buf[0]==5&&udp_buf[1]==5)
  {
    //char SIPdec[20];
    //int Sport;
    //inet_ntop(AF_INET, &sender_sock->sin_addr, SIPdec, 20);
    //Sport = ntohs(sender_sock->sin_port);
    //printf("port %d  ip %s\n", Sport, SIPdec);	
    //domain_ip("www.baidu.com");		
	//broadcast_back(sss);
	
    uint8_t sendbuf[2];
    sendbuf[0] = 6;
    sendbuf[1] = 6;
    sendto(sss, sendbuf, 2, 0, (struct sockaddr*)sender_sock, sizeof(struct sockaddr)); 
  }
}

int keep_running=1;
int run_loop(int BasicSocket)
{
  uint8_t pktbuf[PKTBUF_SIZE]; 
  int rc = -1;
  
  keep_running=1;
  while(keep_running) 
  { 

  rc = SelectRecv(BasicSocket, 3000);
  switch(rc)
  {
    case -1: 
      printf("select error!\n");
      break;
    case 0: 
      //printf("Timeout!\n");
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
}

void stop_loop(int signo) 
{
  keep_running = 0;
}

void *broadcast_main(void* p)
{
  int BasicSock = -1;
	
  BasicSock = open_socket(SPORT_DEFAULT, 1 /*bind ANY*/);
  if( -1 == BasicSock ){
    printf("Failed to open main socket. %s\n", strerror(errno) );
    exit(-2);
  }else{
    printf("supernode is listening on UDP %u (main)\n", SPORT_DEFAULT);
  }
  signal(SIGINT, stop_loop); 

  int c = run_loop(BasicSock);
}


pthread_t pt_broadcast;
int BroadCast_UDP()
{
  pthread_create(&pt_broadcast, 0, broadcast_main, NULL);
  return 0;
}