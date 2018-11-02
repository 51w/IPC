#include <memory.h> 
#include <net/if.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "h264.h" 
#include <arpa/inet.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#define DEFAULT_PORT    1234
#define RECEIVE_BUF_SIZE 1500

void print_time ()
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

int rtpnum = 0;
int nalu_len = 0;
void decode_rtp2h264(unsigned char *rtp_buf, int len, FILE *savefp);
void decode_rtp2h264(unsigned char *rtp_buf, int len, FILE *savefp)
{
    NALU_HEADER *nalu_header;
    FU_HEADER   *fu_header;
    unsigned char h264_nal_header;

    nalu_header = (NALU_HEADER *)&rtp_buf[12];
    if (nalu_header->TYPE == 28) { /* FU-A */
        fu_header = (FU_HEADER *)&rtp_buf[13];

        nalu_len = nalu_len + len - 14;

        if (fu_header->E == 1) { /* end of fu-a */
            fwrite(&rtp_buf[14], 1, len - 14, savefp);

            printf("%3d, len: %6d  ",rtpnum++, nalu_len + 1);  
            printf("nal_unit_type: %x\n", h264_nal_header & 0x1f);
            nalu_len = 0;
        } else if (fu_header->S == 1) { /* start of fu-a */
            // fprintf(savefp, "%c%c%c%c", 0, 0, 0, 1);
            printf("%3d   ",rtpnum);
            print_time();
            fputc(0, savefp);
            fputc(0, savefp);
            fputc(0, savefp);
            fputc(1, savefp);
            h264_nal_header = (fu_header->TYPE & 0x1f) 
                | (nalu_header->NRI << 5)
                | (nalu_header->F << 7);
            fputc(h264_nal_header, savefp);
            fwrite(&rtp_buf[14], 1, len - 14, savefp);
        } else { /* middle of fu-a */
            fwrite(&rtp_buf[14], 1, len - 14, savefp);
        }
    } else { /* nalu */
        // fprintf(savefp, "%c%c%c%c", 0, 0, 0, 1);
        printf("%3d   ",rtpnum);
        print_time();
        fputc(0, savefp);
        fputc(0, savefp);
        fputc(0, savefp);
        fputc(1, savefp); 
        h264_nal_header = (nalu_header->TYPE & 0x1f) 
            | (nalu_header->NRI << 5)
            | (nalu_header->F << 7);
        
        printf("%3d, len: %6d  ",rtpnum++, len - 12);
        printf("nal_unit_type: %x\n", h264_nal_header & 0x1f);

        fputc(h264_nal_header, savefp);
        fwrite(&rtp_buf[13], 1, len - 13, savefp);
    }

    fflush(savefp);
    return;
}

void show_buf(unsigned char *rtp_buf, int len)
{
    int temp = len > 16 ? 16 : len;
    for(int i=0;i<temp;i++)
    {
        printf("%0x ",rtp_buf[i]);
    }
    printf("\n");
}

//usage: %s save_filename [server_IP] [port]\n
int main(int argc, char **argv)
{
    FILE *save_file_fd;
    unsigned short port;
    int socket_s = -1;
    struct sockaddr_in si_me;
    int ret;
    unsigned char buf[RECEIVE_BUF_SIZE];

    if (argc != 4) {
        fprintf(stderr, "usage: %s save_filename [server_IP] [recv_port]\n", 
                argv[0]);
        exit(0);
    }

    //H264输出文件
    save_file_fd = fopen(argv[1], "wb");
    if (!save_file_fd) {
        perror("fopen");
        exit(1);
    }

    const char* serverIP = argv[2];
    port = atoi(argv[3]);

    //init socket
    socket_s = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_s < 0) {
        printf("socket fail!\n");
        exit(1);
    }

    bzero(&si_me, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(56789);
    si_me.sin_addr.s_addr = inet_addr("0.0.0.0");

    if(bind(socket_s,(struct sockaddr *)&si_me,sizeof(struct sockaddr_in))<0) 
	{ 
		fprintf(stderr,"Bind Error:\n"); 
		exit(1); 
	}  

    // if(connect(socket_s, (struct sockaddr*)&si_me,sizeof(si_me)) == -1)
    // {
    //     printf("connect error.\n");
    //         exit(1);
    // }

    //send(socket_s,"connect", 7, 0);
/**TEST
 * 
    ret=recv(socket_s, 
            buf,
            sizeof(buf),
            0);
    buf[ret]='\0';
    printf("recv %s\n",buf);
*/
printf("==============\n");
socklen_t addr_len = sizeof(struct sockaddr_in);
struct sockaddr_in peerRtpAddr = {0};		  
    peerRtpAddr.sin_family = AF_INET;	  
    peerRtpAddr.sin_addr.s_addr = inet_addr("0.0.0.0");
    peerRtpAddr.sin_port = htons(45678);
	

    while(1)
    {
        ret = recv(socket_s, buf, sizeof(buf), 0);
					
		//ret = recvfrom(socket_s, buf, sizeof(buf), 0,
        //                    (struct sockaddr *)&peerRtpAddr, &addr_len);
							
        if (ret < 0) {
            fprintf(stderr, "recv fail\n");
            continue;
        }
        fprintf(stderr, "==================%d\n", ret);
        //show_buf(buf,ret);
        decode_rtp2h264(buf, ret, save_file_fd);
    } /* wile (1) */

    fclose(save_file_fd);
    close(socket_s);

    return 0;
}