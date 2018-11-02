#include<stdio.h>  
#include<stdlib.h>  
#include<sys/time.h>
#include<unistd.h>
#include<string.h>  
#include<sys/socket.h>
#include<sys/types.h>  
#include<netdb.h> //gethostbyname
#include<errno.h>
#include<arpa/inet.h>
#include<signal.h>
#include<netinet/in.h>
  
#include"ping.h"  
#define IP_HSIZE sizeof(struct iphdr)   /*定义IP_HSIZE为ip头部长度*/  
#define IPVERSION  4   /*定义IPVERSION为4，指出用ipv4*/ 

/*设置的时间是一个结构体，倒计时设置，重复倒时，超时值设为1秒*/  
struct itimerval val_alarm={.it_interval.tv_sec = 1,      
                               .it_interval.tv_usec=0,  
                               .it_value.tv_sec=0,  
                               .it_value.tv_usec=1  
                                };  
  
int main(int argc,char **argv) 
{  
    struct hostent *host; 
    int on =1;  

    if(argc<2)
    {
        printf("Usage: %s hostname\n",argv[0]);  
        exit(1);  
    }  
  
    //getnameinfo   gethostbyname
    if((host=gethostbyname(argv[1]))==NULL)
    { 
        perror("can not understand the host name");
        exit(1);  
    } 
    hostname=argv[1];
      

    memset(&dest,0,sizeof dest);  /*将dest中前sizeof(dest)个字节替换为0并返回s,此处为初始化,给最大内存清零*/  
    dest.sin_family=PF_INET;  /*PF_INET为IPV4，internet协议，在<netinet/in.h>中，地址族*/   
    dest.sin_port=ntohs(0);   /*端口号,ntohs()返回一个以主机字节顺序表达的数。*/  
    dest.sin_addr=*(struct in_addr *)host->h_addr_list[0];/*host->h_addr_list[0]是地址的指针.返回IP地址，初始化*/  
  
    if((sockfd = socket(PF_INET,SOCK_RAW,IPPROTO_ICMP))<0)
    { //PF_INEI套接字协议族，SOCK_RAW套接字类型，IPPROTO_ICMP使用协议，调用socket函数来创建一个能够进行网络通信的套接字。这里判断是否创建成功
        perror("raw socket created error");  
        exit(1);  
    }  
  
    setsockopt(sockfd,IPPROTO_IP,IP_HDRINCL,&on,sizeof(on));  /*设置当前套接字选项特定属性值，sockfd套接字，IPPROTO_IP协议层为IP层，IP_HDRINCL套接字选项条目，套接字接收缓冲区指针，sizeof(on)缓冲区长度的长度*/  
  
    setuid(getuid());/*getuid()函数返回一个调用程序的真实用户ID,setuid()是让普通用户可以以root用户的角色运行只有root帐号才能运行的程序或命令。*/  
    pid=getpid(); /*getpid函数用来取得目前进程的进程识别码*/  
    set_sighandler();/*对信号处理*/  
    printf("Ping %s(%s): %d bytes data in ICMP packets.\n",  argv[1],inet_ntoa(dest.sin_addr),datalen); 
  
    if((setitimer(ITIMER_REAL,&val_alarm,NULL))==-1)
        bail("setitimer fails.");  
  
    recv_reply();/*接收ping应答*/  
  
    return 0;  
} 
  
void send_ping(void)  
{  
    struct iphdr *ip_hdr;   /*iphdr为IP头部结构体*/  
    struct icmphdr *icmp_hdr;   /*icmphdr为ICMP头部结构体*/  
    int len;  
    int len1;  
        /*ip头部结构体变量初始化*/  
    ip_hdr=(struct iphdr *)sendbuf; /*字符串指针*/     
    ip_hdr->hlen=sizeof(struct iphdr)>>2;  /*头部长度*/  
    ip_hdr->ver=IPVERSION;   /*版本*/  
    ip_hdr->tos=0;   /*服务类型*/  
    ip_hdr->tot_len=IP_HSIZE+ICMP_HSIZE+datalen; /*报文头部加数据的总长度*/  
    ip_hdr->id=0;    /*初始化报文标识*/  
    ip_hdr->frag_off=0;  /*设置flag标记为0*/  
    ip_hdr->protocol=IPPROTO_ICMP;/*运用的协议为ICMP协议*/  
    ip_hdr->ttl=255; /*一个封包在网络上可以存活的时间*/  
    ip_hdr->daddr=dest.sin_addr.s_addr;  /*目的地址*/  
    len1=ip_hdr->hlen<<2;  /*ip数据长度*/  
    /*ICMP头部结构体变量初始化*/  
    icmp_hdr=(struct icmphdr *)(sendbuf+len1);  /*字符串指针*/  
    icmp_hdr->type=8;    /*初始化ICMP消息类型type*/  
    icmp_hdr->code=0;    /*初始化消息代码code*/  
    icmp_hdr->icmp_id=pid;   /*把进程标识码初始给icmp_id*/  
    icmp_hdr->icmp_seq=nsent++;  /*发送的ICMP消息序号赋值给icmp序号*/      
    memset(icmp_hdr->data,0xff,datalen);  /*将datalen中前datalen个字节替换为0xff并返回icmp_hdr-dat*/    
  
    gettimeofday((struct timeval *)icmp_hdr->data,NULL); /* 获取当前时间*/  
  
    len=ip_hdr->tot_len; /*报文总长度赋值给len变量*/  
    icmp_hdr->checksum=0;    /*初始化*/  
    icmp_hdr->checksum=checksum((u8 *)icmp_hdr,len);  /*计算校验和*/  
  
    sendto(sockfd,sendbuf,len,0,(struct sockaddr *)&dest,sizeof (dest)); /*经socket传送数据*/  
}

//接收程序发出的ping命令的应答 
void recv_reply()  
{  
    int n,len;  
    int errno;  
  
    n=nrecv=0;  
    len=sizeof(from);   /*发送ping应答消息的主机IP*/  
  
    while(nrecv<4)
    {  
        if((n=recvfrom(sockfd,recvbuf,sizeof recvbuf,0,(struct sockaddr *)&from,&len))<0)
        { /*经socket接收数据,如果正确接收返回接收到的字节数，失败返回0.*/  
            if(errno==EINTR)  /*EINTR表示信号中断*/  
            continue;  
            bail("recvfrom error");  
        }  

        gettimeofday(&recvtime,NULL);   /*记录收到应答的时间*/  
        if(handle_pkt())    /*接收到错误的ICMP应答信息*/  

        continue;  
        nrecv++;  
    }  

    get_statistics(nsent,nrecv);     /*统计ping命令的检测结果*/  
} 

 //计算校验和 
u16 checksum(u8 *buf,int len)  
{  
    u32 sum=0;  
    u16 *cbuf;  
    cbuf=(u16 *)buf;  

    while(len>1)
    {  
        sum+=*cbuf++;  
        len-=2;  
    }  

    if(len)  
    sum+=*(u8 *)cbuf;  

    sum=(sum>>16)+(sum & 0xffff);  
    sum+=(sum>>16);  

    return ~sum;  
}

 //ICMP应答消息处理  
int handle_pkt()  
{  
    struct iphdr *ip;  
    struct icmphdr *icmp;  

    int ip_hlen;  
    u16 ip_datalen;
    double rtt;
    struct timeval *sendtime;  
    ip=(struct iphdr *)recvbuf;  

    ip_hlen=ip->hlen << 2;  
    ip_datalen=ntohs(ip->tot_len)-ip_hlen;  
    icmp=(struct icmphdr *)(recvbuf+ip_hlen);  

    if(checksum((u8 *)icmp,ip_datalen)) //计算校验和* 
        return -1;  

    if(icmp->icmp_id!=pid)  
        return -1;  

    sendtime=(struct timeval *)icmp->data;  
    rtt=((&recvtime)->tv_sec-sendtime->tv_sec)*1000+((&recvtime)->tv_usec-sendtime->tv_usec)/1000.0;
    
    printf("%d bytes from %s:icmp_seq=%u ttl=%d rtt=%.3f ms\n",  ip_datalen,
        inet_ntoa(from.sin_addr), icmp->icmp_seq, ip->ttl,  rtt);

    return 0;  
} 

//设置信号处理程序
void set_sighandler()  
{  
    act_alarm.sa_handler=alarm_handler;  
    if(sigaction(SIGALRM,&act_alarm,NULL)==-1)  
        bail("SIGALRM handler setting fails.");  
    //sigaction()会依参数signum指定的信号编号来设置该信号的处理函数。参数signum指所要捕获信号或忽略的信号，
    //&act代表新设置的信号共用体，NULL代表之前设置的信号处理结构体。这里判断对信号的处理是否成功.  

    act_int.sa_handler=int_handler;  
    if(sigaction(SIGINT,&act_int,NULL)==-1)  
        bail("SIGALRM handler setting fails.");  
} 

//统计ping命令的检测结果 
void get_statistics(int nsent,int nrecv)  
{  
    printf("--- %s ping statistics ---\n",inet_ntoa(dest.sin_addr)); //*将网络地址转换成“.”点隔的字符串格式。
    printf("%d packets transmitted, %d received, %0.0f%% ""packet loss\n",  
           nsent,nrecv,1.0*(nsent-nrecv)/nsent*100);  
}  

void bail(const char * on_what)  
{  
    fputs(strerror(errno),stderr); 
    fputs(":",stderr);  
    fputs(on_what,stderr);  
    fputc('\n',stderr); //送一个字符到一个流中  
    exit(1);  
}  
  

void int_handler(int sig)  
{  
    get_statistics(nsent,nrecv);//统计ping命令的检测结果 
    close(sockfd);              //关闭网络套接字  
    exit(1);  
} 

//SIGALRM（终止进程）处理程序
void alarm_handler(int signo)  
{  
    send_ping();
}