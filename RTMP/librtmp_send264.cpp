#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "librtmp_send264.h"
#include "librtmp/rtmp.h"

#define RTMP_HEAD_SIZE   (sizeof(RTMPPacket)+RTMP_MAX_HEADER_SIZE)
RTMP* m_pRtmp;
NaluUnit sps;
NaluUnit pps;

int RTMP264_Connect(const char* url)  
{
	m_pRtmp = RTMP_Alloc();
	RTMP_Init(m_pRtmp);
	//设置URL
	if(RTMP_SetupURL(m_pRtmp,(char*)url) == FALSE)
	{
		RTMP_Free(m_pRtmp);
		printf("URL Not found..");
		return -1;
	}
	//设置可写,即发布流,这个函数必须在连接前使用,否则无效
	RTMP_EnableWrite(m_pRtmp);
	//连接服务器
	if(RTMP_Connect(m_pRtmp, NULL) == FALSE) 
	{
		RTMP_Free(m_pRtmp);
		return -1;
	} 

	//连接流
	if(RTMP_ConnectStream(m_pRtmp,0) == FALSE)
	{
		RTMP_Close(m_pRtmp);
		RTMP_Free(m_pRtmp);
		return -1;
	}
	return 0;  
}

int RTMP264_Close()  
{  
	if(m_pRtmp)  
	{  
		RTMP_Close(m_pRtmp);  
		RTMP_Free(m_pRtmp);  
		m_pRtmp = NULL;  
	}
	return 0;
}

int SendPacket(unsigned int nPacketType, char *data, unsigned int size, unsigned int nTimestamp)  
{
	RTMPPacket* packet;
	/*分配包内存和初始化,len为包体长度*/
	packet = (RTMPPacket *)malloc(RTMP_HEAD_SIZE+size);
	memset(packet,0,RTMP_HEAD_SIZE);
	/*包体内存*/
	packet->m_body = (char *)packet + RTMP_HEAD_SIZE;
	packet->m_nBodySize = size;
	memcpy(packet->m_body,data,size);
	packet->m_hasAbsTimestamp = 0;
	packet->m_packetType = nPacketType; /*此处为类型有两种一种是音频,一种是视频*/
	packet->m_nInfoField2 = m_pRtmp->m_stream_id;
	packet->m_nChannel = 0x04;

	packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
	if (RTMP_PACKET_TYPE_AUDIO ==nPacketType && size !=4)
	{
		packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
	}
	packet->m_nTimeStamp = nTimestamp;
	/*发送*/
	int nRet =0;
	if (RTMP_IsConnected(m_pRtmp))
	{
		nRet = RTMP_SendPacket(m_pRtmp,packet,TRUE); /*TRUE为放进发送队列,FALSE是不放进发送队列,直接发送*/
	}
	/*释放内存*/
	free(packet);
	return nRet;  
}

int SendVideoSpsPps(char *pps, int pps_len, char * sps, int sps_len)
{
	RTMPPacket * packet=NULL;//rtmp包结构
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

	/*AVCDecoderConfigurationRecord*/
	body[i++] = 0x01;
	body[i++] = sps[1];
	body[i++] = sps[2];
	body[i++] = sps[3];
	body[i++] = 0xff;

	/*sps*/
	body[i++]   = 0xe1;
	body[i++] = (sps_len >> 8) & 0xff;
	body[i++] = sps_len & 0xff;
	memcpy(&body[i],sps,sps_len);
	i +=  sps_len;

	/*pps*/
	body[i++]   = 0x01;
	body[i++] = (pps_len >> 8) & 0xff;
	body[i++] = (pps_len) & 0xff;
	memcpy(&body[i],pps,pps_len);
	i +=  pps_len;

	packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
	packet->m_nBodySize = i;
	packet->m_nChannel = 0x04;
	packet->m_nTimeStamp = 0;
	packet->m_hasAbsTimestamp = 0;
	packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
	packet->m_nInfoField2 = m_pRtmp->m_stream_id;

	/*调用发送接口*/
	int nRet = RTMP_SendPacket(m_pRtmp,packet,TRUE);
	free(packet);    //释放内存
	return nRet;
}

int SendH264Packet(char *data, unsigned int size, int bIsKeyFrame, unsigned int nTimeStamp)
{
	if(data == NULL || size<11)
	{  
		return -1;  
	}  

	char *body = (char*)malloc(size+9);  
	memset(body,0,size+9);

	int i = 0; 
	if(bIsKeyFrame){  
		body[i++] = 0x17;// 1:Iframe  7:AVC   
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
		SendVideoSpsPps(pps.data, pps.size, sps.data, sps.size);
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

int free_h264(std::vector<NaluUnit> &input)
{
	for(int i=0; i<input.size(); i++)
	free(input[i].data);

	if(sps.size > 0) free(sps.data);
	if(pps.size > 0) free(pps.data);
}

int parse_h264(std::vector<NaluUnit> &input)
{
	FILE *fp = fopen("cuc_ieschool.h264", "rb");
	fseek(fp, 0L, SEEK_END);
	int length = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	char *fbuff = (char *)malloc(length);
	fread(fbuff, 1, length, fp);

	
	int Spos = 0;
	int Epos = 0;
	while(Spos < length)
	{
		if(fbuff[Spos++] == 0x00 && fbuff[Spos++] == 0x00)
		{
			if(fbuff[Spos++] == 0x01)
				goto gotnal_head;
			else
			{
				Spos--;
				if(fbuff[Spos++] == 0x00 && fbuff[Spos++] == 0x01)
					goto gotnal_head;
				else
					continue;
			}
		}
		else
		{
			continue;
		}
		
	gotnal_head:
		Epos = Spos;
		//int size = 0;
		NaluUnit NALdata;
		while(Epos < length)
		{
			if(fbuff[Epos++] == 0x00 && fbuff[Epos++] == 0x00)
			{
				if(fbuff[Epos++] == 0x01)
				{
					NALdata.size = (Epos-3)-Spos;
					break;
				}
				else
				{
					Epos--;
					if(fbuff[Epos++] == 0x00 && fbuff[Epos++] == 0x01)
					{	
						NALdata.size = (Epos-4)-Spos;
						break;
					}
				}
			}
		}
		if(Epos >= length)
		{
			NALdata.size = Epos - Spos;
			NALdata.type = fbuff[Spos]&0x1f;
			NALdata.data = (char*)malloc(NALdata.size);
			memcpy(NALdata.data, fbuff+Spos, NALdata.size);
			if(NALdata.type == 7)		 sps = NALdata;
			else if(NALdata.type == 8) 	 pps = NALdata;
			else	input.push_back(NALdata);

			break;
		}

		NALdata.type = fbuff[Spos]&0x1f;
		NALdata.data = (char*)malloc(NALdata.size);
		memcpy(NALdata.data, fbuff+Spos, NALdata.size);
		if(NALdata.type == 7)		 sps = NALdata;
		else if(NALdata.type == 8) 	 pps = NALdata;
		else if(NALdata.type == 6) 	 ;//printf("%s", NALdata.data);
		else	input.push_back(NALdata);

		Spos = Epos - 4;
	}

	free(fbuff);
	fclose(fp);
}