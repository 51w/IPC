#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <librtmp/rtmp.h>

int main()
{
  RTMP* rtmpdump = RTMP_Alloc();
  RTMP_Init(rtmpdump);
  
  char *url = "rtmp://148.70.17.190:1935/live/720p";
  //char *url = "rtmp://192.168.1.7:1935/hls";
  if(RTMP_SetupURL(rtmpdump,(char*)url)==FALSE)
  {
    printf("URL Not found..\n");
    goto rtmpdump_destroy;
  }
  
  if(RTMP_Connect(rtmpdump, NULL)==FALSE) 
  {
	printf("RTMP_Connect ERROR.\n");
    RTMP_Close(rtmpdump);
    goto rtmpdump_destroy;
  }
  
  if(RTMP_ConnectStream(rtmpdump,0)==FALSE)
  {
	printf("RTMP_ConnectStream ERROR.\n");
    RTMP_Close(rtmpdump);
    goto rtmpdump_destroy;
  }
  
  int nRead = 0;
  char data[1024*1024];
  int size= 1024*1024;
  while(nRead=RTMP_Read(rtmpdump, data, size))
  {
	unsigned char *ptr = data;
	int len = nRead;
	while(len>4)
	{
	  if(ptr[0]=='F'&&ptr[1]=='L'&&ptr[2]=='V')
	  {
		//printf("FLV.\n");
	    ptr += 13;  len -= 13;
	  }
	  else if(ptr[0]==9)
	  {
	    ptr += 11;  len -= 11; //1Type 3Size 3Time 4Stream
		
		int video_type  = ptr[0]&0x0f;
		int frame_type = (ptr[0]&0xf0)>>4;
		int packet_type = ptr[1];
		int nal_size = (ptr[5]<<24) + (ptr[6]<<16) + (ptr[7]<<8) + ptr[8];
		ptr += 9;  len -= 9;
		
		if(packet_type==1&&frame_type==2) //P-frame
		{
		  printf("P=======%d\n", nal_size);
		  ptr += nal_size;  len -= nal_size;
		  ptr += 4;  len -= 4; 
		}
        else if(packet_type==1&&frame_type==1)
		{
		  printf("I=======%d\n", nal_size);
		  ptr += nal_size;  len -= nal_size;
          ptr += 4;  len -= 4; 		  
		}
		else if(packet_type==0&&frame_type==1)
		{
		  int sps_len = (ptr[2]<<8) + ptr[3];
		  ptr += 4;  len -= 4; 
		  ptr += sps_len;  len -= sps_len; 
		  
		  int pps_len = (ptr[1]<<8) + ptr[2];
		  ptr += 3;  len -= 3; 
		  ptr += pps_len;  len -= pps_len; 
		  ptr += 4;  len -= 4; 
		  
		  printf("PS=======%d %d\n", sps_len, pps_len);
		}
	  }
	  else len = 0;
	}
  }
  
  printf("================\n");

rtmpdump_destroy:
  RTMP_Free(rtmpdump);
  rtmpdump = NULL;
  
  return 0;
}