#include <stdio.h>
#include <memory.h> 
#include <net/if.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#define DEFAULT_PORT    1234
#define RECEIVE_BUF_SIZE 8000

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

    if (argc != 2) {
        fprintf(stderr, "usage: %s save_filename\n", argv[0]);
        //exit(0);
    }

    //H264Êä³öÎÄ¼þ
    save_file_fd = fopen(argv[1], "wb");
    if (!save_file_fd) {
        perror("fopen");
        //exit(1);
    }

    //init socket
    socket_s = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_s < 0) {
        printf("socket fail!\n");
        //exit(1);
    }

    bzero(&si_me, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(56789);
    si_me.sin_addr.s_addr = inet_addr("0.0.0.0");

    if(bind(socket_s,(struct sockaddr *)&si_me,sizeof(struct sockaddr_in))<0) 
	{ 
		fprintf(stderr,"Bind Error:\n"); 
		//exit(1); 
	}  

    while(1)
    {
        ret = recv(socket_s, buf, sizeof(buf), 0);
        if (ret < 0)
		{
            fprintf(stderr, "recv fail\n");
            continue;
        }
        
        show_buf(buf,ret);
		printf("=============%d\n", ret);
        //decode_rtp2h264(buf, ret, save_file_fd);
    }

    fclose(save_file_fd);
    close(socket_s);

    return 0;
}