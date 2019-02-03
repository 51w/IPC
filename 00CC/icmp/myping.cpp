#include <stdio.h>  
#include <strings.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>  
#include <sys/types.h>  
#include <netinet/in.h>  
#include <netinet/ip.h>  
#include <netinet/ip_icmp.h>  
#include <netdb.h>  
#include <sys/time.h>  
#include <arpa/inet.h>
#define PACKET_SIZE     4096  
  
//效验算法（百度下有注释，但是还是看不太明白）  
static unsigned short CalChkSum( unsigned short *addr, int len )  
{  
    int nleft=len;  
    int sum=0;  
    unsigned short *w=addr;  
    unsigned short answer=0;  
      
    while(nleft > 1)  
    {  
        sum 	+= *w++;  
        nleft 	-= 2;  
    }  
      
    if( nleft == 1)  
    {         
        *(unsigned char *)(&answer) = *(unsigned char *)w;  
        sum += answer;  
    }  
      
    sum 	= (sum >> 16) + (sum & 0xffff);  
    sum 	+= (sum >> 16);  
    answer 	= ~sum;  
      
    return answer;  
}  
/*
* fn: 	执行一次ping 动作
* ips: 	被ping 的ip 地址
* timeout: 超时时间(ms)
*/
int MyPing( char *ips, int timeout)    
{        
	struct timeval *tval;      
	struct timeval timeo;    
	pid_t pid;    
	fd_set readfds;    
    struct ip *iph;    
    struct icmp *icmp; 
    char *fromIp; // MyPing 后返回包中的的ip   
	int sockfd;
	int maxfds = 0;    
	struct sockaddr_in addr;    
	struct sockaddr_in from;  
	int ret = -1, n;
	char sendpacket[PACKET_SIZE];    
    char recvpacket[PACKET_SIZE];      
      
	// 设定Ip信息    
	bzero( &addr,sizeof(addr) );    
	addr.sin_family 		= AF_INET;    
	addr.sin_addr.s_addr 	= inet_addr( ips );     
    
    // 取得socket    
    sockfd = socket( AF_INET, SOCK_RAW, IPPROTO_ICMP );    
    if( sockfd < 0 )    
    {    
        printf( "ip: %s, socket error!\r\n", ips );    
        return -1;    
    }  
    
    // 设定TimeOut时间    
    timeo.tv_sec 	= timeout / 1000;    
    timeo.tv_usec 	= timeout % 1000;    
      
    if( setsockopt( sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeo, sizeof(timeo) ) == -1 )    
    {    
        printf( "ip:%s,setsockopt errorr!\r\n", ips );    
        return -1;    
    }        
    // 设定Ping包    
    memset(sendpacket, 0, sizeof(sendpacket));          
    // 取得PID，作为Ping的Sequence ID    
    pid		= getpid();       
    icmp	= (struct icmp*)sendpacket;    
    icmp->icmp_type		= ICMP_ECHO;  //回显请求  
    icmp->icmp_code		= 0;    
    icmp->icmp_cksum	= 0;    
    icmp->icmp_seq		= 0;    
    icmp->icmp_id		= pid;   
    tval= (struct timeval *)icmp->icmp_data;    
    gettimeofday( tval, NULL );    
    icmp->icmp_cksum	= CalChkSum( (unsigned short *)icmp, sizeof(struct icmp) );  //校验  
      
    // 发包    
    n = sendto(sockfd, (char *)&sendpacket, sizeof(struct icmp), 0, (struct sockaddr *)&addr, sizeof(addr));    
    if( n < 1 )    
    {    
        printf( "ip:%s,sendto error!\r\n", ips );    
        return -1;    
    }    
      
    // 接收  
    // 由于可能接受到其他Ping的应答消息，所以这里要用循环    
    while(1)    
    {    
        // 设定TimeOut时间，这次才是真正起作用的    
        FD_ZERO( &readfds );    
        FD_SET( sockfd, &readfds ); 
        maxfds = sockfd + 1;    
        n = select( maxfds, &readfds, NULL, NULL, &timeo );    
        if( n <= 0 )    
        {    
            printf( "failed:%s ip:%s, select()!\r\n", __FUNCTION__,  ips );    
            ret = -1;  
            break;
        }    
          
        // 接受    
        memset( recvpacket, 0, sizeof(recvpacket) );    
        int fromlen = sizeof(from);    
        n = recvfrom(sockfd, recvpacket, sizeof(recvpacket), 0, 
        				(struct sockaddr *)&from, (socklen_t *)&fromlen);
        if( n < 1 ) 
        {  
        	ret = -1;
            break;    
        }              
       
        fromIp = (char *)inet_ntoa( from.sin_addr );    
        // 判断是否是自己Ping的回复    
        if( strcmp(fromIp, ips) != 0 )    
        {    
            printf( "NowPingip:%s Fromip:%s\nNowPingip is not same to Fromip,so MyPing wrong!\n", ips, fromIp );    
            ret = -1;
            break;  
        }    
          
        iph 	= (struct ip *)recvpacket;           
        icmp	= (struct icmp *)( recvpacket + (iph->ip_hl<<2) );    
          
        // printf( "ip:%s\n,icmp->icmp_type:%d\n,icmp->icmp_id:%d\n",
        //						ips, icmp->icmp_type, icmp->icmp_id );    
        // 判断Ping回复包的状态    
        if( icmp->icmp_type == ICMP_ECHOREPLY && icmp->icmp_id == pid )   //ICMP_ECHOREPLY回显应答  
        {    
            // 正常就退出循环    
	   		ret = 0;
           	break;    
        }    
        else    
        {    
            // 否则继续等    
            continue;    
        }    
    } // while(1
    
	close( sockfd );
    return ret;
} 


int main(int argc,char **argv)
{
	//printf(">>>>>>>>>start\n");
	int ret = MyPing(argv[1],5000);
	
	printf(">>>>>>>>>%d\n", ret);
	return 0;
}
