#include <stdio.h>   
#include <string.h> 
#include <stdlib.h> 
#include <pthread.h>  
#include <sys/socket.h>   
#include <netinet/in.h>   
#include <netinet/ip.h>   
#include <netinet/ip_icmp.h>   
#include <netdb.h>  
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
 
#define PACKET_SIZE     4096   
#define ERROR           0   
#define SUCCESS         1   
  
//效验算法（百度下有注释，但是还是看不太明白）   
unsigned short cal_chksum(unsigned short *addr, int len)  
{  
    int nleft=len;  
    int sum=0;  
    unsigned short *w=addr;  
    unsigned short answer=0;  
      
    while(nleft > 1)  
    {  
        sum += *w++;  
        nleft -= 2;  
    }  
      
    if( nleft == 1)  
    {         
        *(unsigned char *)(&answer) = *(unsigned char *)w;  
        sum += answer;  
    }  
      
    sum = (sum >> 16) + (sum & 0xffff);  
    sum += (sum >> 16);  
    answer = ~sum;  
      
    return answer;  
}  
// Ping函数   
int ping( char *ips, int timeout)    
{        
    struct timeval *tval;          
    int maxfds = 0;    
    fd_set readfds;    
      
    struct sockaddr_in addr;    
    struct sockaddr_in from;    
    // 设定Ip信息     
    bzero(&addr,sizeof(addr));    
    addr.sin_family = AF_INET;    
    addr.sin_addr.s_addr = inet_addr(ips);    
      
    int sockfd;    
    // 取得socket     
    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);    
    if (sockfd < 0)    
    {    
        printf("ip:%s,socket error\n",ips);    
        return ERROR;    
    }    
      
    struct timeval timeo;    
    // 设定TimeOut时间     
    timeo.tv_sec = timeout / 1000;    
    timeo.tv_usec = timeout % 1000;    
      
    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeo, sizeof(timeo)) == -1)    
    {    
        printf("ip:%s,setsockopt error\n",ips);    
        return ERROR;    
    }    
      
    char sendpacket[PACKET_SIZE];    
    char recvpacket[PACKET_SIZE];    
    // 设定Ping包     
    memset(sendpacket, 0, sizeof(sendpacket));    
      
    pid_t pid;    
    // 取得PID，作为Ping的Sequence ID     
    pid=getpid();    
      
    struct ip *iph;    
    struct icmp *icmp;    
      
    
    icmp=(struct icmp*)sendpacket;    
    icmp->icmp_type=ICMP_ECHO;  //回显请求   
    icmp->icmp_code=0;    
    icmp->icmp_cksum=0;    
    icmp->icmp_seq=0;    
    icmp->icmp_id=pid;   
    tval= (struct timeval *)icmp->icmp_data;    
    gettimeofday(tval,NULL);    
    icmp->icmp_cksum=cal_chksum((unsigned short *)icmp,sizeof(struct icmp));  //校验   
      
    int n;    
    // 发包     
    n = sendto(sockfd, (char *)&sendpacket, sizeof(struct icmp), 0, (struct sockaddr *)&addr, sizeof(addr));    
    if (n < 1)    
    {    
        printf("ip:%s,sendto error\n",ips);    
        return ERROR;    
    }    
      
    // 接受     
    // 由于可能接受到其他Ping的应答消息，所以这里要用循环     
    while(1)    
    {    
        // 设定TimeOut时间，这次才是真正起作用的     
        FD_ZERO(&readfds);    
        FD_SET(sockfd, &readfds);    
        maxfds = sockfd + 1;    
        n = select(maxfds, &readfds, NULL, NULL, &timeo);    
        if (n <= 0)    
        {    
            printf("ip:%s,Time out error\n",ips);    
            close(sockfd);    
            return ERROR;    
        }    
          
        // 接受     
        memset(recvpacket, 0, sizeof(recvpacket));    
        int fromlen = sizeof(from);    
        n = recvfrom(sockfd, recvpacket, sizeof(recvpacket), 0, (struct sockaddr *)&from, (socklen_t *)&fromlen);    
        if (n < 1) {    
            break;    
        }    
          
       
        char *from_ip = (char *)inet_ntoa(from.sin_addr);    
            // 判断是否是自己Ping的回复     
        if (strcmp(from_ip,ips) != 0)    
        {    
            printf("NowPingip:%s Fromip:%s\nNowPingip is not same to Fromip,so ping wrong!\n",ips,from_ip);    
            return ERROR;  
        }    
          
        iph = (struct ip *)recvpacket;    
          
        icmp=(struct icmp *)(recvpacket + (iph->ip_hl<<2));    
          
        printf("ip:%s\n,icmp->icmp_type:%d\n,icmp->icmp_id:%d\n",ips,icmp->icmp_type,icmp->icmp_id);    
        // 判断Ping回复包的状态     
        if (icmp->icmp_type == ICMP_ECHOREPLY && icmp->icmp_id == pid)   //ICMP_ECHOREPLY回显应答   
        {    
            // 正常就退出循环     
            break;    
        }    
        else    
        {    
            // 否则继续等     
            continue;    
        }    
    }    
} 
  
int main(int argc,char **argv)  
{      
    if(ping(argv[1],10000))  
    {  
        printf("Ping succeed!\n");  
    }  
    else  
    {  
        printf("Ping wrong!\n");  
    }  
      
}  