#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <librtmp/rtmp.h>
#include "rtmpstream.h"
#include "bufpool2.h"

#define RTMP_HEAD_SIZE (sizeof(RTMPPacket)+RTMP_MAX_HEADER_SIZE)
RTMP* RtmpHandle;
char spsdata[50];
char ppsdata[50];
int  spslen = 0;
int  ppslen = 0;

void setsps(char *data, int size)
{
  spslen = size;
  memcpy(spsdata, data, spslen);
  
  //int i;
  //for(i=0; i<size; i++)
  //  printf("%d ", (char)data[i]);
  //printf("\n\n");
}
void setpps(char *data, int size)
{
  ppslen = size;
  memcpy(ppsdata, data, ppslen);
  
  //int i;
  //for(i=0; i<size; i++)
  //  printf("%d ", (char)data[i]); 
  //printf("\n\n");
}

int rtmp_connect(char* url)  
{
  printf("[%s]\n", url);
  RtmpHandle = RTMP_Alloc();
  RTMP_Init(RtmpHandle);
  
  if(RTMP_SetupURL(RtmpHandle, url) == FALSE)
  {
    RTMP_Free(RtmpHandle);
    printf("URL Not found..\n");
    return -1;
  }
  
  //设置可写,即发布流,这个函数必须在连接前使用,否则无效
  RTMP_EnableWrite(RtmpHandle);
  if(RTMP_Connect(RtmpHandle, NULL) == FALSE) 
  {
    //RTMP_Free(RtmpHandle);
	printf("RTMP_Connect ERROR.\n");
    return -1;
  }

  if(RTMP_ConnectStream(RtmpHandle,0) == FALSE)
  {
    RTMP_Close(RtmpHandle);
    RTMP_Free(RtmpHandle);
	printf("RTMP_ConnectStream ERROR.\n");
    return -1;
  }
  return 0;  
}

int rtmp_close()
{
  if(RtmpHandle)
  {  
    RTMP_Close(RtmpHandle);  
    RTMP_Free(RtmpHandle);  
    RtmpHandle = NULL;  
  }
  return 0;
}

int SendPacket(unsigned int nPacketType, char *data, unsigned int size, unsigned int nTimestamp) 
{
  RTMPPacket* packet;
  packet = (RTMPPacket *)malloc(RTMP_HEAD_SIZE+size);
  memset(packet,0,RTMP_HEAD_SIZE);
	
  packet->m_body = (char *)packet + RTMP_HEAD_SIZE;
  packet->m_nBodySize = size;
  memcpy(packet->m_body,data,size);
  packet->m_hasAbsTimestamp = 0;
  packet->m_packetType = nPacketType; //音频,视频
  packet->m_nInfoField2 = RtmpHandle->m_stream_id;
  packet->m_nChannel = 0x04;

  packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
  if (RTMP_PACKET_TYPE_AUDIO ==nPacketType && size !=4)
  {
    packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
  }
  packet->m_nTimeStamp = nTimestamp;

  int nRet =0;
  if (RTMP_IsConnected(RtmpHandle))
  {//TRUE为放进发送队列,FALSE是不放进发送队列,直接发送
    nRet = RTMP_SendPacket(RtmpHandle, packet, TRUE); 
  }

  free(packet);
  return nRet;  
}

int SendVideoSpsPps(char *pps, int pps_len, char * sps, int sps_len)
{
  RTMPPacket * packet=NULL;
  char * body=NULL;
  int i;
  packet = (RTMPPacket *)malloc(RTMP_HEAD_SIZE+1024);
  //RTMPPacket_Reset(packet);//重置packet状态
  memset(packet,0,RTMP_HEAD_SIZE+1024);
  packet->m_body = (char *)packet + RTMP_HEAD_SIZE;
  body = (char *)packet->m_body;
  i = 0;
  body[i++] = 0x17;
  body[i++] = 0x00;

  body[i++] = 0x00;
  body[i++] = 0x00;
  body[i++] = 0x00;

  body[i++] = 0x01;
  body[i++] = sps[1];
  body[i++] = sps[2];
  body[i++] = sps[3];
  body[i++] = 0xff;

  //sps
  body[i++] = 0xe1;
  body[i++] = (sps_len >> 8) & 0xff;
  body[i++] = sps_len & 0xff;
  memcpy(&body[i],sps,sps_len);
  i += sps_len;

  //pps
  body[i++] = 0x01;
  body[i++] = (pps_len >> 8) & 0xff;
  body[i++] = (pps_len) & 0xff;
  memcpy(&body[i],pps,pps_len);
  i += pps_len;

  packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
  packet->m_nBodySize = i;
  packet->m_nChannel = 0x04;
  packet->m_nTimeStamp = 0;
  packet->m_hasAbsTimestamp = 0;
  packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
  packet->m_nInfoField2 = RtmpHandle->m_stream_id;

  //调用发送接口
  int nRet = RTMP_SendPacket(RtmpHandle, packet, TRUE);
  free(packet); //释放内存
  return nRet;
}

static int spssend = 0;
int SendH264Packet(char *data, unsigned int size, int bIsKeyFrame, unsigned int nTimeStamp)
{
  if(data==NULL||size<11)  return -1;  

  char *body = (char*)malloc(size+9);  
  memset(body,0,size+9);

  int i = 0; 
  if(bIsKeyFrame)
  {
	body[i++] = 0x17;// 1:Iframe 7:AVC   
	body[i++] = 0x01;// AVC NALU   
	body[i++] = 0x00;  
	body[i++] = 0x00;  
	body[i++] = 0x00;  

	// NALU size   
	body[i++] = size>>24 &0xff;  
	body[i++] = size>>16 &0xff;  
	body[i++] = size>>8 &0xff;  
	body[i++] = size&0xff;
	// NALU data   
	memcpy(&body[i],data,size);
	if(spssend == 0)
	{
	  SendVideoSpsPps(ppsdata, ppslen, spsdata, spslen);
	  spssend = 1;
	}
  }else{
	body[i++] = 0x27;// 2:Pframe  7:AVC   
	body[i++] = 0x01;// AVC NALU   
	body[i++] = 0x00;  
	body[i++] = 0x00;  
	body[i++] = 0x00;  

	// NALU size   
	body[i++] = size>>24 &0xff;  
	body[i++] = size>>16 &0xff;  
	body[i++] = size>>8 &0xff;  
	body[i++] = size&0xff;
	// NALU data   
	memcpy(&body[i],data,size);  
  }  

  int bRet = SendPacket(RTMP_PACKET_TYPE_VIDEO, body, i+size, nTimeStamp);

  free(body);
  return bRet;
}

int streamrun = 0;
void *StreamMain(void* p)
{
  printf("========rtmpdump=======%d\n", RTMP_HEAD_SIZE);
  //rtmp_connect("rtmp://192.168.1.7:1935/hls");
  //rtmp_connect("rtmp://192.168.1.8:1935/live/720p");
  rtmp_connect("rtmp://148.70.17.190:1935/live/720p");
  
  char stream[500*1024];
  BufData data;
  data.bptr = stream;
  data.len  = 0;
  
  unsigned int nTimeStamp = 0;
  streamrun = 1;
  while(streamrun)
  {
    if(PullBuf(&data) && data.len>0)
    {
	  printf("===========%d=========\n", data.len);
		
	  int NalType = data.bptr[0]&0x1f;
	  if(NalType == 7)      setsps(data.bptr, data.len);
	  else if(NalType == 8) setpps(data.bptr, data.len);
	  else if(NalType == 5)
	  {
		SendH264Packet(data.bptr, data.len, 1, nTimeStamp);
		nTimeStamp += 33;
	  }
	  else if(NalType == 1)
	  {
		SendH264Packet(data.bptr, data.len, 0, nTimeStamp);
		nTimeStamp += 33;
	  }
    }
  }
  
  rtmp_close();
}

pthread_t pt_rtmp;
int rtmpstream_run()
{
  pthread_create(&pt_rtmp, 0, StreamMain, NULL);
}

int rtmpstream_exit()
{
  streamrun = 0;
  usleep(200*1000);
}