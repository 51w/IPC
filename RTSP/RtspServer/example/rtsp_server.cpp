// PHZ
// RTSP服务器Demo

#include "RtspServer.h"
#include "xop.h"
#include "xop/NetInterface.h"
#include <thread>
#include <memory>
#include <iostream>
#include <string>
#include <vector>

typedef struct _NaluUnit
{
	unsigned int size;
	int   type;
	char *data;
}NaluUnit;
int parse_h264(std::vector<NaluUnit> &input);

using namespace xop;

// 负责音视频数据转发的线程函数
//void snedFrame(xop::RtspServer* rtspServer, xop::MediaSessionId sessionId, int& clients);
void *sendff(void *pArgs);
xop::RtspServer *server;
int *cclient;
xop::MediaSessionId sessionId;

int main(int agrc, char **argv)
{	
    XOP_Init(); //WSAStartup

    int clients = 0; // 记录当前客户端数量
	cclient = &clients;
    std::string ip = xop::NetInterface::getLocalIPAddress(); //获取网卡ip地址
    std::string rtspUrl;
    
    std::shared_ptr<xop::EventLoop> eventLoop(new xop::EventLoop());  
    xop::RtspServer server11(eventLoop.get(), ip, 8554);  //创建一个RTSP服务器
	server = &server11;

    xop::MediaSession *session = xop::MediaSession::createNew("live"); //创建一个媒体会话, url: rtsp://ip/live
    rtspUrl = "rtsp://" + ip + "/" + session->getRtspUrlSuffix();
    
    // 添加音视频流到媒体会话, track0:h264, track1:aac
    session->addMediaSource(xop::channel_0, xop::H264Source::createNew()); 
    session->addMediaSource(xop::channel_1, xop::AACSource::createNew(44100,2));
    // session->startMulticast(); // 开启组播(ip,端口随机生成), 默认使用 RTP_OVER_UDP, RTP_OVER_RTSP

    // 设置通知回调函数。 在当前会话中, 客户端连接或断开会通过回调函数发起通知
    session->setNotifyCallback([&clients, &rtspUrl](xop::MediaSessionId sessionId, uint32_t numClients) {
        clients = numClients; //获取当前媒体会话客户端数量
        std::cout << "[" << sessionId << " " << rtspUrl << "]" << " Online: " << clients << std::endl;
    });

    std::cout << rtspUrl << std::endl;
        
    //xop::MediaSessionId sessionId = server.addMeidaSession(session); //添加session到RtspServer后, session会失效
    sessionId = server11.addMeidaSession(session);
	//server.removeMeidaSession(sessionId); //取消会话, 接口线程安全
         
    //std::thread t1(snedFrame, &server, sessionId, std::ref(clients)); //开启负责音视频数据转发的线程
    //t1.detach();
	pthread_t id;
	pthread_create(&id, NULL, sendff, NULL);
   
    eventLoop->loop(); //主线程运行 RtspServer 

    getchar();
    return 0;
}

// 负责音视频数据转发的线程函数
//void snedFrame(xop::RtspServer* rtspServer, xop::MediaSessionId sessionId, int& clients)
void *sendff(void *pArgs)
{
    std::vector<NaluUnit> input; 
    parse_h264(input);
    int npos = 0;

    while(1)
    {
        if(cclient > 0) // 媒体会话有客户端在线, 发送音视频数据
        {
            {
                /*                     
                    //获取一帧 H264, 打包
                    xop::AVFrame videoFrame = {0};
                    videoFrame.size = 100000;  // 视频帧大小 
                    videoFrame.timestamp = H264Source::getTimeStamp(); // 时间戳, 建议使用编码器提供的时间戳
                    videoFrame.buffer.reset(new char[videoFrame.size]);
                    memcpy(videoFrame.buffer.get(), 视频帧数据, videoFrame.size);					
                   
                    rtspServer->pushFrame(sessionId, xop::channel_0, videoFrame); //送到服务器进行转发, 接口线程安全
                */
                xop::AVFrame videoFrame = {0};
                videoFrame.size = input[npos].size;
				//videoFrame.type = input[npos].type;
                videoFrame.timestamp = H264Source::getTimeStamp();
                videoFrame.buffer.reset(new char[videoFrame.size]);
                memcpy(videoFrame.buffer.get(), input[npos].data, videoFrame.size);

                server->pushFrame(sessionId, xop::channel_0, videoFrame);

                npos++;
                if(npos == input.size()) npos = 0;
            }
                    
            {				
                /*
                    //获取一帧 AAC, 打包
                    xop::AVFrame audioFrame = {0};
                    audioFrame.size = 500;  // 音频帧大小 
                    audioFrame.timestamp = AACSource::getTimeStamp(44100); // 时间戳
                    audioFrame.buffer.reset(new char[audioFrame.size]);
                    memcpy(audioFrame.buffer.get(), 音频帧数据, audioFrame.size);

                    rtspServer->pushFrame(sessionId, xop::channel_1, audioFrame); //送到服务器进行转发, 接口线程安全
                */
            }		
        }

        xop::Timer::sleep(30); // 实际使用需要根据帧率计算延时!
    }
}



int parse_h264(std::vector<NaluUnit> &input)
{
	FILE *fp = fopen("../../h264/11.h264", "rb");
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
			input.push_back(NALdata);

			break;
		}

		NALdata.type = fbuff[Spos]&0x1f;
		NALdata.data = (char*)malloc(NALdata.size);
		memcpy(NALdata.data, fbuff+Spos, NALdata.size);
		if(NALdata.type != 6)	input.push_back(NALdata);

		Spos = Epos - 4;
	}

	free(fbuff);
	fclose(fp);
	return 0;
}