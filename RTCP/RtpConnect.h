#include <unistd.h> 
#include <string.h> 
#include <net/if.h>  
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <memory>
using namespace std;

#define RTP_HEADER_SIZE   	   12
#define MAX_RTP_PAYLOAD_SIZE   1420//1460  1500-20-12-8
#define RTP_VERSION			   2
#define RTP_TCP_HEAD_SIZE	   4

typedef struct _RTP_header
{
#ifdef BIGENDIAN//defined(sun) || defined(__BIG_ENDIAN) || defined(NET_ENDIAN)
    unsigned char version:2;
    unsigned char padding:1;
    unsigned char extension:1;
    unsigned char csrc:4;
    unsigned char marker:1;
    unsigned char payload:7;
#else
    unsigned char csrc:4;
    unsigned char extension:1;
    unsigned char padding:1;
    unsigned char version:2;
    unsigned char payload:7;
    unsigned char marker:1;
#endif
    unsigned short seq;
    unsigned int   ts;
    unsigned int   ssrc;
} RtpHeader;

typedef struct __AVFrame
{	
    __AVFrame() 
    {
        size = 0;
        type = 0;
        timestamp = 0;
    }

    __AVFrame(uint32_t size) :buffer(new char[size])
    {
        this->size = size;
        type = 0;
        timestamp = 0;
    }

    std::shared_ptr<char> buffer; /* 帧数据 */
    uint32_t size;				  /* 帧大小 */
    uint8_t  type;				  /* 帧类型 */	
    uint32_t timestamp;		  	  /* 时间戳 */
} AVFrame;

typedef std::shared_ptr<char> RtpPacketPtr;

class RtpConnect
{
public:
    RtpConnect(int port = 45678);
    virtual ~RtpConnect();

    void SetpeerRtpAdd(string ip, int port = 45678);

    bool pushFrame(AVFrame& frame);

    static uint32_t getTimeStamp();

    bool sendFrame(uint8_t frameType, RtpPacketPtr& rtpPkt, uint32_t pktSize, uint8_t last, uint32_t ts);

    void setFrameType(uint8_t frameType=0);
    void setRtpHeader(RtpPacketPtr& rtpPkt, uint8_t last, uint32_t ts);
    int sendRtpPacket(RtpPacketPtr& rtpPkt, uint32_t pktSize);

private:
    int sockfd;
    struct sockaddr_in peerRtpAddr;

    RtpHeader rtpHeader;
    unsigned int packetSeq;
};