#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <memory.h> 
#include <net/if.h>
#include <netinet/in.h>
#include <pthread.h> 
#include "h264.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <sys/time.h>
#include <time.h>
#include <unistd.h>
  
#define  UDP_MAX_SIZE 1400
#define SEND_BUF_SIZE 1500
#define BUFFER_SIZE 200000
//#define SSRC_NUM 10

void print_time()
{
	struct timeval tv;
	struct tm* ptm;
	char time_string[40];
	long milliseconds;

	/* 获得日期时间，并转化为 struct tm。 */
	gettimeofday (&tv, NULL);
	ptm = localtime (&tv.tv_sec);
	/* 格式化日期和时间，精确到秒为单位。*/
	strftime (time_string, sizeof (time_string), "%Y-%m-%d %H:%M:%S", ptm);
	/* 从微秒计算毫秒。*/
	milliseconds = tv.tv_usec / 1000;
  	/* 以秒为单位打印格式化后的时间日期，小数点后为毫秒。*/
	printf ("%s.%03ld\n", time_string, milliseconds);
}

typedef struct  
{  
    int startcodeprefix_len;      //! 4 for parameter sets and first slice in picture, 3 for everything else (suggested)  
    unsigned len;                 //! Length of the NAL unit (Excluding the start code, which does not belong to the NALU)  
    unsigned max_size;            //! Nal Unit Buffer size  
    int forbidden_bit;            //! should be always FALSE  
    int nal_reference_idc;        //! NALU_PRIORITY_xxxx  
    int nal_unit_type;            //! NALU_TYPE_xxxx      
    char *buf;                    //! contains the first byte followed by the EBSP  
    unsigned short lost_packets;  //! true, if packet loss is detected  
} NALU_t;  
  
FILE *bits = NULL;                //!< the bit stream file  
static int FindStartCode2(unsigned char *Buf);//查找开始字符0x000001  
static int FindStartCode3(unsigned char *Buf);//查找开始字符0x00000001  
  
  
static int info2=0, info3=0;  
RTP_FIXED_HEADER *rtp_hdr;  
  
NALU_HEADER     *nalu_hdr;  
FU_INDICATOR    *fu_ind;  
FU_HEADER       *fu_hdr;  
 
  
//为NALU_t结构体分配内存空间  
NALU_t *AllocNALU(int buffersize)  
{  
    NALU_t *n;  
  
    if ((n = (NALU_t*)calloc (1, sizeof (NALU_t))) == NULL)  
    {  
        printf("AllocNALU: n");  
        exit(0);  
    }  
  
    n->max_size=buffersize;  
  
    if ((n->buf = (char*)calloc (buffersize, sizeof (char))) == NULL)  
    {  
        free (n);  
        printf ("AllocNALU: n->buf");  
        exit(0);  
    }  
  
    return n;  
}  
  
//释放  
void FreeNALU(NALU_t *n)  
{  
    if (n)  
    {  
        if (n->buf)  
        {  
            free(n->buf);  
            n->buf=NULL;  
        }  
        free (n);  
    }  
}  
  
void OpenBitstreamFile (const char *fn)  
{  
    if (NULL == (bits=fopen(fn, "rb")))  
    {  
        printf("open file error\n");  
        exit(0);  
    }  
}  
  
//这个函数输入为一个NAL结构体，主要功能为得到一个完整的NALU并保存在NALU_t的buf中，  
//获取他的长度，填充F,IDC,TYPE位。  
//并且返回两个开始字符之间间隔的字节数，即包含有前缀的NALU的长度  
int GetAnnexbNALU (NALU_t *nalu)  
{  
    int pos = 0;  
    int StartCodeFound, rewind;  
    unsigned char *Buf;  
  
    if ((Buf = (unsigned char*)calloc (nalu->max_size , sizeof(char))) == NULL)   
        printf ("GetAnnexbNALU: Could not allocate Buf memory\n");  
  
    nalu->startcodeprefix_len=3;//初始化码流序列的开始字符为3个字节  
  
    if (3 != fread (Buf, 1, 3, bits))//从码流中读3个字节  
    {  
        free(Buf);  
        return 0;  
    }  
    info2 = FindStartCode2 (Buf);//判断是否为0x000001   
    if(info2 != 1)   
    {  
        //如果不是，再读一个字节  
        if(1 != fread(Buf+3, 1, 1, bits))//读一个字节  
        {  
            free(Buf);  
            return 0;  
        }  
        info3 = FindStartCode3 (Buf);//判断是否为0x00000001  
        if (info3 != 1)//如果不是，返回-1  
        {   
            free(Buf);  
            return -1;  
        }  
        else   
        {  
            //如果是0x00000001,得到开始前缀为4个字节  
            pos = 4;  
            nalu->startcodeprefix_len = 4;  
        }  
    }  
    else  
    {  
        //如果是0x000001,得到开始前缀为3个字节  
        nalu->startcodeprefix_len = 3;  
        pos = 3;  
    }  
    //查找下一个开始字符的标志位  
    StartCodeFound = 0;  
    info2 = 0;  
    info3 = 0;  
  
    while (!StartCodeFound)  
    {  
        if (feof (bits))//判断是否到了文件尾  
        {  
            nalu->len = pos-nalu->startcodeprefix_len;  
            memcpy (nalu->buf, &Buf[nalu->startcodeprefix_len], nalu->len);       
            nalu->forbidden_bit = nalu->buf[0] & 0x80; //1 bit  
            nalu->nal_reference_idc = nalu->buf[0] & 0x60; // 2 bit  
            nalu->nal_unit_type = nalu->buf[0] & 0x1f;// 5 bit  
            free(Buf);  
            return pos;  
        }  
        Buf[pos++] = fgetc (bits);//读一个字节到BUF中  
        info3 = FindStartCode3(&Buf[pos-4]);//判断是否为0x00000001  
        if(info3 != 1)  
            info2 = FindStartCode2(&Buf[pos-3]);//判断是否为0x000001  
        StartCodeFound = (info2 == 1 || info3 == 1);  
    }  
  
    // Here, we have found another start code (and read length of startcode bytes more than we should  
    // have.  Hence, go back in the file  
    rewind = (info3 == 1)? -4 : -3;  
  
    if (0 != fseek (bits, rewind, SEEK_CUR))//把文件指针指向前一个NALU的末尾  
    {  
        free(Buf);  
        printf("GetAnnexbNALU: Cannot fseek in the bit stream file");  
    }  
  
    // Here the Start code, the complete NALU, and the next start code is in the Buf.    
    // The size of Buf is pos, pos+rewind are the number of bytes excluding the next  
    // start code, and (pos+rewind)-startcodeprefix_len is the size of the NALU excluding the start code  
  
    nalu->len = (pos+rewind)-nalu->startcodeprefix_len;  
    memcpy (nalu->buf, &Buf[nalu->startcodeprefix_len], nalu->len);//拷贝一个完整NALU，不拷贝起始前缀0x000001或0x00000001  
    nalu->forbidden_bit = nalu->buf[0] & 0x80;        //1 bit  
    nalu->nal_reference_idc = nalu->buf[0] & 0x60;    //2 bit  
    nalu->nal_unit_type = nalu->buf[0] & 0x1f;  //5 bit  
    free(Buf);  
  
    return (pos+rewind);//返回两个开始字符之间间隔的字节数，即包含有前缀的NALU的长度  
}  
  
static int FindStartCode2 (unsigned char *Buf)  
{  
    if(Buf[0]!=0 || Buf[1]!=0 || Buf[2] !=1) return 0; //判断是否为0x000001,如果是返回1  
    else return 1;  
}  
  
static int FindStartCode3 (unsigned char *Buf)  
{  
    if(Buf[0]!=0 || Buf[1]!=0 || Buf[2] !=0 || Buf[3] !=1) return 0;//判断是否为0x00000001,如果是返回1  
    else return 1;  
}  
  
int rtpnum = 0;  
  
//输出NALU长度和TYPE  
void dump(NALU_t *n)  
{  
    if (!n)return;  
    printf("%3d, len: %6d  ",rtpnum++, n->len);  
    printf("nal_unit_type: %x\n", n->nal_unit_type);  
}  
struct sockaddr_in serveraddr, clientaddr;
socklen_t addr_len;
int sockfd;
//int sin_size; 

//usage: %s <H264_filename> [port]
int main(int argc, char* argv[])  
{
    if(argc != 3){
        fprintf(stderr, "usage: %s <H264_filename> [port]\n", 
                argv[0]);
        exit(0);
    }

    const char* raw_file = argv[1];
    //const char* DEST_IP = argv[];
    unsigned short DEST_PORT = atoi(argv[2]);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd == -1)
    {
        perror("socket");
        exit(1);
    }

    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(DEST_PORT);
    //接收来自任意IP发来的数据
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    addr_len = sizeof(struct sockaddr_in);

    //绑定socket
    if(bind(sockfd,(const struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
    {
        perror("bind");
        exit(1);
    }

    //发包rtp包初始化
    //unsigned int addr;
    OpenBitstreamFile(raw_file);  
    NALU_t *n;
    n = AllocNALU(BUFFER_SIZE);//为结构体nalu_t及其成员buf分配空间。返回值为指向nalu_t存储空间的指针   
    char* nalu_payload;    
    char sendbuf[SEND_BUF_SIZE];  
    unsigned short seq_num =0;  
    int bytes=0;  
    float framerate=25;  
    unsigned int timestamp_increse = 3600;
    //timestamp_increse=(unsigned int)(90000.0 / framerate);
    unsigned int ts_current=0; 
    unsigned int SSRC_NUM = 10; 

    int req_len = 0;
    char req_buf[50];
    //一直监听端口
    while(1)
    {
        bzero(req_buf, sizeof(req_buf));
        bzero(&clientaddr,sizeof(clientaddr));
        req_len = recvfrom(sockfd, req_buf, sizeof(req_buf), 0,
                            (struct sockaddr *)&clientaddr, &addr_len);
        if(req_len < 0)
        {
            perror("recvfrom error\n");
            exit(1);
        }

        req_buf[req_len] = '\0';
        //printf("req_len = %d\n",req_len);
        printf("Connect from client %s, port is %d.\n",
                inet_ntoa(clientaddr.sin_addr), htons(clientaddr.sin_port));
        
        //sendto(sockfd, "SBSB", 4, 0, (struct sockaddr*)&clientaddr, addr_len);      
        /*
        if(connect(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1)
        {
            printf("connect error.\n");
            exit(1);
        }*/
        int isCircle = 1;
        while(isCircle > 0)
        {
            isCircle--;
            while(!feof(bits))   
            {
                GetAnnexbNALU(n);//每执行一次，文件的指针指向本次找到的NALU的末尾，下一个位置即为下个NALU的起始码0x000001  
                dump(n);//输出NALU长度和TYPE  

                ts_current+=timestamp_increse;

                //当一个NALU小于1400字节的时候，采用一个单RTP包发送  
                if(n->len <= UDP_MAX_SIZE){  
                    memset(sendbuf,0,SEND_BUF_SIZE);//清空sendbuf；此时会将上次的时间戳清空，因此需要ts_current来保存上次的时间戳值 
                    //rtp固定包头，为12字节,该句将sendbuf[0]的地址赋给rtp_hdr，以后对rtp_hdr的写入操作将直接写入sendbuf。  
                    rtp_hdr =(RTP_FIXED_HEADER *)&sendbuf[0];
                    //设置RTP HEADER
                    rtp_hdr->csrc_len = 0;
                    rtp_hdr->extension = 0;
                    rtp_hdr->padding = 0;  
                    rtp_hdr->version = 2;   //版本号，此版本固定为2    
                    rtp_hdr->payload = H264;//负载类型号
                    rtp_hdr->marker  = 0;   //标志位，由具体协议规定其值。
                    rtp_hdr->seq_no = htons(++seq_num%UINT16_MAX); //序列号，每发送一个RTP包增1  
                    rtp_hdr->timestamp =htonl(ts_current);
                    rtp_hdr->ssrc = htonl(SSRC_NUM);//随机指定为10，并且在本RTP会话中全局唯一  
            

                    //设置NALU HEADER,并将这个HEADER填入sendbuf[12]  
                    nalu_hdr =(NALU_HEADER *)&sendbuf[12]; //将sendbuf[12]的地址赋给nalu_hdr，之后对nalu_hdr的写入就将写入sendbuf中；  
                    nalu_hdr->F = (n->forbidden_bit) >> 7;  
                    nalu_hdr->NRI = (n->nal_reference_idc) >> 5;//有效数据在n->nal_reference_idc的第6，7位，需要右移5位才能将其值赋给nalu_hdr->NRI。  
                    nalu_hdr->TYPE = n->nal_unit_type;
                    //printf("%d %d\n",nalu_hdr->TYPE,n->nal_unit_type);  
        
                    nalu_payload=&sendbuf[13];//同理将sendbuf[13]赋给nalu_payload  
                    memcpy(nalu_payload,n->buf+1,n->len-1);//去掉nalu头的nalu剩余内容写入sendbuf[13]开始的字符串。  
        
                    bytes=n->len + 12 ;  //获得sendbuf的长度,为nalu的长度（包含NALU头但除去起始前缀）加上rtp_header的固定长度12字节   
                    
                    //printf("%3d, len: %6d  ",rtpnum++, bytes);
                    printf("%3d   ",rtpnum-1);
                    print_time();
                    sendto(sockfd, sendbuf, bytes, 0, (struct sockaddr*)&clientaddr, addr_len);
                    //send(sockfd, sendbuf, bytes, 0);
                }else{
                //分片发送
                    memset(sendbuf,0,SEND_BUF_SIZE);
                    //rtp固定包头，为12字节,该句将sendbuf[0]的地址赋给rtp_hdr，以后对rtp_hdr的写入操作将直接写入sendbuf。  
                    rtp_hdr =(RTP_FIXED_HEADER *)&sendbuf[0];
                    //设置RTP HEADER
                    rtp_hdr->csrc_len = 0;
                    rtp_hdr->extension = 0;
                    rtp_hdr->padding = 0;  
                    rtp_hdr->version = 2;   //版本号，此版本固定为2    
                    rtp_hdr->payload = H264;//负载类型号
                    rtp_hdr->marker  = 0;   //标志位，由具体协议规定其值。
                    rtp_hdr->seq_no = htons(++seq_num%UINT16_MAX); //序列号，每发送一个RTP包增1  
                    rtp_hdr->timestamp=htonl(ts_current);
                    rtp_hdr->ssrc    = htonl(SSRC_NUM);//随机指定为10，并且在本RTP会话中全局唯一  
            

                    int packetNum = n->len/UDP_MAX_SIZE;  
                    if (n->len%UDP_MAX_SIZE != 0)  
                        packetNum ++;  
        
                    int lastPackSize = n->len - (packetNum-1)*UDP_MAX_SIZE;  
                    int packetIndex = 1; 
        
                    //发送第一个的FU，S=1，E=0，R=0  

                    //设置FU INDICATOR,并将这个HEADER填入sendbuf[12]  
                    fu_ind =(FU_INDICATOR*)&sendbuf[12]; //将sendbuf[12]的地址赋给fu_ind，之后对fu_ind的写入就将写入sendbuf中；  
                    fu_ind->F=n->forbidden_bit>>7;  
                    fu_ind->NRI=n->nal_reference_idc>>5;  
                    fu_ind->TYPE=28;  
        
                    //设置FU HEADER,并将这个HEADER填入sendbuf[13]  
                    fu_hdr =(FU_HEADER*)&sendbuf[13];  
                    fu_hdr->S=1;  
                    fu_hdr->E=0;  
                    fu_hdr->R=0;  
                    fu_hdr->TYPE=n->nal_unit_type;  
        
                    nalu_payload=&sendbuf[14];//同理将sendbuf[14]赋给nalu_payload  
                    memcpy(nalu_payload,n->buf+1,UDP_MAX_SIZE-1);//去掉NALU头  
                    bytes=UDP_MAX_SIZE-1+14;//获得sendbuf的长度,为nalu的长度（除去起始前缀和NALU头）加上rtp_header，fu_ind，fu_hdr的固定长度14字节   
                    
                    //printf("%3d, len: %6d  ",rtpnum++, bytes);
                    printf("%3d   ",rtpnum-1);
                    print_time();
                    
                    sendto(sockfd, sendbuf, bytes, 0, (struct sockaddr*)&clientaddr, addr_len);
                    //send(sockfd, sendbuf, bytes, 0);

                    //发送中间的FU，S=0，E=0，R=0  
                    for(packetIndex = 2; packetIndex < packetNum;packetIndex++)  
                    {
                        memset(sendbuf,0,SEND_BUF_SIZE);
                        //rtp固定包头，为12字节,该句将sendbuf[0]的地址赋给rtp_hdr，以后对rtp_hdr的写入操作将直接写入sendbuf。  
                        rtp_hdr =(RTP_FIXED_HEADER *)&sendbuf[0];
                        //设置RTP HEADER
                        rtp_hdr->csrc_len = 0;
                        rtp_hdr->extension = 0;
                        rtp_hdr->padding = 0;  
                        rtp_hdr->version = 2;   //版本号，此版本固定为2    
                        rtp_hdr->payload = H264;//负载类型号
                        rtp_hdr->marker  = 0;   //标志位，由具体协议规定其值。
                        rtp_hdr->seq_no = htons(++seq_num%UINT16_MAX); //序列号，每发送一个RTP包增1  
                        rtp_hdr->timestamp=htonl(ts_current);
                        rtp_hdr->ssrc    = htonl(SSRC_NUM);//随机指定为10，并且在本RTP会话中全局唯一  

                        //设置FU INDICATOR,并将这个HEADER填入sendbuf[12]  
                        fu_ind =(FU_INDICATOR*)&sendbuf[12]; //将sendbuf[12]的地址赋给fu_ind，之后对fu_ind的写入就将写入sendbuf中；  
                        fu_ind->F=n->forbidden_bit>>7;  
                        fu_ind->NRI=n->nal_reference_idc>>5;  
                        fu_ind->TYPE=28;  
        
                        //设置FU HEADER,并将这个HEADER填入sendbuf[13]  
                        fu_hdr =(FU_HEADER*)&sendbuf[13];  
                        fu_hdr->S=0;  
                        fu_hdr->E=0;  
                        fu_hdr->R=0;  
                        fu_hdr->TYPE=n->nal_unit_type;  
        
                        nalu_payload=&sendbuf[14];//同理将sendbuf[14]的地址赋给nalu_payload  
                        memcpy(nalu_payload,n->buf+(packetIndex-1)*UDP_MAX_SIZE,UDP_MAX_SIZE);//去掉起始前缀的nalu剩余内容写入sendbuf[14]开始的字符串。  
                        bytes=UDP_MAX_SIZE+14;//获得sendbuf的长度,为nalu的长度（除去原NALU头）加上rtp_header，fu_ind，fu_hdr的固定长度14字节  
                        //send( socket1, sendbuf, bytes, 0 );//发送rtp包 
                        //printf("%3d, len: %6d  ",rtpnum++, bytes);
                        sendto(sockfd, sendbuf, bytes, 0, (struct sockaddr*)&clientaddr, addr_len);              
                        //send(sockfd, sendbuf, bytes, 0);
                    }  

                    memset(sendbuf,0,SEND_BUF_SIZE);
                    //rtp固定包头，为12字节,该句将sendbuf[0]的地址赋给rtp_hdr，以后对rtp_hdr的写入操作将直接写入sendbuf。  
                    rtp_hdr =(RTP_FIXED_HEADER *)&sendbuf[0];
                    //设置RTP HEADER
                    rtp_hdr->csrc_len = 0;
                    rtp_hdr->extension = 0;
                    rtp_hdr->padding = 0;  
                    rtp_hdr->version = 2;   //版本号，此版本固定为2    
                    rtp_hdr->payload = H264;//负载类型号
                    rtp_hdr->marker  = 1;   //标志位，由具体协议规定其值。
                    rtp_hdr->seq_no = htons(++seq_num%UINT16_MAX); //序列号，每发送一个RTP包增1  
                    rtp_hdr->timestamp=htonl(ts_current);
                    rtp_hdr->ssrc    = htonl(SSRC_NUM);//随机指定为10，并且在本RTP会话中全局唯一  

                    //发送最后一个的FU，S=0，E=1，R=0  

                    //设置FU INDICATOR,并将这个HEADER填入sendbuf[12]  
                    fu_ind =(FU_INDICATOR*)&sendbuf[12]; //将sendbuf[12]的地址赋给fu_ind，之后对fu_ind的写入就将写入sendbuf中；  
                    fu_ind->F=n->forbidden_bit>>7;  
                    fu_ind->NRI=n->nal_reference_idc>>5;  
                    fu_ind->TYPE=28;  
        
                    //设置FU HEADER,并将这个HEADER填入sendbuf[13]  
                    fu_hdr =(FU_HEADER*)&sendbuf[13];  
                    fu_hdr->S=0;  
                    fu_hdr->E=1;  
                    fu_hdr->R=0;  
                    fu_hdr->TYPE=n->nal_unit_type;  
        
                    nalu_payload=&sendbuf[14];//同理将sendbuf[14]的地址赋给nalu_payload  
                    memcpy(nalu_payload,n->buf+(packetIndex-1)*UDP_MAX_SIZE,lastPackSize);//将nalu最后剩余的-1(去掉了一个字节的NALU头)字节内容写入sendbuf[14]开始的字符串。  
                    bytes=lastPackSize+14;//获得sendbuf的长度,为剩余nalu的长度l-1加上rtp_header，FU_INDICATOR,FU_HEADER三个包头共14字节   
                    
                    sendto(sockfd, sendbuf, bytes, 0, (struct sockaddr*)&clientaddr, addr_len);
                    //send(sockfd, sendbuf, bytes, 0);
                }
                usleep(20000);//延迟20ms
            }//while(!feof(bits))
            rewind(bits);
            rtpnum = 0;
            //ts_current = 0;
            if(isCircle == 0)
            {
                printf("--------是否继续循环发送？-请输入循环次数:(0为退出循环)\n");
                scanf("%d",&isCircle);
                fflush(stdin);
            }
        }
        //disconnect()
        printf("-------------------等待下一个client请求\n");
    }

    FreeNALU(n);  
    close(sockfd);
    return 0;  
}