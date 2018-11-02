#include "RtpConnect.h"
#include <chrono>

RtpConnect::RtpConnect(int port)
{
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in addr = {0};		  
    addr.sin_family = AF_INET;	  
    addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    addr.sin_port = htons(port);

    if(bind(sockfd, (struct sockaddr*)&addr, sizeof addr) == -1)
    {
        cout << "Sockfd bind error!" << endl;
    }

    int size = 50*1024;
    setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (char *)&size, sizeof(size));
    cout << "[RTP ==> " << port << "]" << endl;

    rtpHeader.version = RTP_VERSION;
    packetSeq = 1; //rd()&0xffff;
    rtpHeader.ssrc = htonl(10);
    rtpHeader.payload = 96;
}

RtpConnect::~RtpConnect()
{
    close(sockfd);
}

uint32_t RtpConnect::getTimeStamp()
{
    auto timePoint = chrono::time_point_cast<chrono::microseconds>(chrono::high_resolution_clock::now());
    return (uint32_t)((timePoint.time_since_epoch().count() + 500) / 1000 * 90 );
}


void RtpConnect::SetpeerRtpAdd(string ip, int port)
{
    peerRtpAddr.sin_family = AF_INET;	  
    peerRtpAddr.sin_addr.s_addr = inet_addr(ip.c_str());
    peerRtpAddr.sin_port = htons(port);
}

bool RtpConnect::pushFrame(AVFrame& frame)
{
    char *frameBuf  = frame.buffer.get();
    uint32_t frameSize = frame.size;

    if(frame.timestamp == 0)
        frame.timestamp = getTimeStamp();

    if(frameSize <= MAX_RTP_PAYLOAD_SIZE)
    {
        RtpPacketPtr rtpPkt(new char[1500]);
        memcpy(rtpPkt.get()+4+RTP_HEADER_SIZE, frameBuf, frameSize); // 预留12字节 rtp header

        sendFrame(frame.type, rtpPkt, frameSize+4+RTP_HEADER_SIZE, 1, frame.timestamp);
    }
    else
    {
        char FU_A[2] = {0};

        // 分包参考live555
        FU_A[0] = (frameBuf[0] & 0xE0) | 28;
        FU_A[1] = 0x80 | (frameBuf[0] & 0x1f);

        frameBuf  += 1;
        frameSize -= 1;

        while(frameSize + 2 > MAX_RTP_PAYLOAD_SIZE)
        {
            RtpPacketPtr rtpPkt(new char[1500]);
            rtpPkt.get()[RTP_HEADER_SIZE+4] = FU_A[0];
            rtpPkt.get()[RTP_HEADER_SIZE+5] = FU_A[1];
            memcpy(rtpPkt.get()+4+RTP_HEADER_SIZE+2, frameBuf, MAX_RTP_PAYLOAD_SIZE-2);

            sendFrame(frame.type, rtpPkt, 4+RTP_HEADER_SIZE+MAX_RTP_PAYLOAD_SIZE, 0, frame.timestamp);

            frameBuf  += MAX_RTP_PAYLOAD_SIZE - 2;
            frameSize -= MAX_RTP_PAYLOAD_SIZE - 2;

            FU_A[1] &= ~0x80;
        }

        {
			//RtpPacketPtr rtpPkt((char*)xop::Alloc(1500), xop::Free);
            RtpPacketPtr rtpPkt(new char[1500]);
            FU_A[1] |= 0x40;
            rtpPkt.get()[RTP_HEADER_SIZE+4] = FU_A[0];
            rtpPkt.get()[RTP_HEADER_SIZE+5] = FU_A[1];
            memcpy(rtpPkt.get()+4+RTP_HEADER_SIZE+2, frameBuf, frameSize);

            sendFrame(frame.type, rtpPkt, 4+RTP_HEADER_SIZE+2+frameSize, 1, frame.timestamp);
        }
    }

    return true;
}

bool RtpConnect::sendFrame(uint8_t frameType, RtpPacketPtr& rtpPkt, uint32_t pktSize, uint8_t last, uint32_t ts)
{
    setFrameType(frameType);
    setRtpHeader(rtpPkt, last, ts);
    sendRtpPacket(rtpPkt, pktSize);

    return true;
}

void RtpConnect::setFrameType(uint8_t frameType)
{
    //cout << frameType << endl;
}
void RtpConnect::setRtpHeader(RtpPacketPtr& rtpPkt, uint8_t last, uint32_t ts)
{
    rtpHeader.marker = last;
    rtpHeader.ts = htonl(ts);
    rtpHeader.seq = htons(packetSeq++);
    memcpy(rtpPkt.get()+4, &rtpHeader, RTP_HEADER_SIZE);
}
int RtpConnect::sendRtpPacket(RtpPacketPtr& rtpPkt, uint32_t pktSize)
{
    int ret = sendto(sockfd, rtpPkt.get()+4,
                    pktSize-4, 0, (struct sockaddr *)&peerRtpAddr,
                    sizeof(struct sockaddr_in));
    if(ret < 0)
    {
        //teardown();
        return -1;
    }

    return ret;
}