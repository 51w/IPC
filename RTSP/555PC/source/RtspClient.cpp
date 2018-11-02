#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "GroupsockHelper.hh"
UsageEnvironment* env;
portNumBits tunnelOverHTTPPortNum = 0;
const char * url="rtsp://127.0.0.1:1935/vod/Extremists.m4v";
#if defined(__WIN32__) || defined(_WIN32)
#define snprintf _snprintf  
#endif  
int main(int argc,const char ** argv)  
{  
    //创建BasicTaskScheduler对象  
    TaskScheduler* scheduler = BasicTaskScheduler::createNew();  
    //创建BisicUsageEnvironment对象  
    env = BasicUsageEnvironment::createNew(*scheduler);  
    //创建RTSPClient对象  
    RTSPClient * rtspClient= RTSPClient::createNew(*env);  
    //由RTSPClient对象向服务器发送OPTION消息并接受回应  
    char* optionsResponse=rtspClient->sendOptionsCmd(url);  
    delete [] optionsResponse;  
    //产生SDPDescription字符串（由RTSPClient对象向服务器发送DESCRIBE消息并接受回应，根据回应的信息产生SDPDescription字符串，其中包括视音频数据的协议和解码器类型）  
    char* sdpDescription =rtspClient->describeURL(url);  
    //创建MediaSession对象（根据SDPDescription在MediaSession中创建和初始化MediaSubSession子会话对象）  
    MediaSession* session = MediaSession::createNew(*env, sdpDescription);  
    delete[] sdpDescription;  


    MediaSubsessionIterator iter(*session);  
    MediaSubsession *subsession;  
    while ((subsession = iter.next()) != NULL) {  
        // Creates a "RTPSource" for this subsession. (Has no effect if it's  
        // already been created.)  Returns True iff this succeeds.  
        if (!subsession->initiate()) {  
            *env << "Unable to create receiver for /"" << subsession->mediumName()  
                << "/" << subsession->codecName()  
                << "/" subsession: " << env->getResultMsg() << "/n";  
        } else {
            *env << "Created receiver for /"" << subsession->mediumName()  
                << "/" << subsession->codecName()  
                << "/" subsession (client ports " << subsession->clientPortNum()  
                << "-" << subsession->clientPortNum()+1 << ")/n";  
            if (subsession->rtpSource() != NULL) {  
                // Because we're saving the incoming data, rather than playing  
                // it in real time, allow an especially large time threshold  
                // (1 second) for reordering misordered incoming packets:  
                unsigned const thresh = 1000000; // 1 second  
                subsession->rtpSource()->setPacketReorderingThresholdTime(thresh);  
                // Set the RTP source's OS socket buffer size as appropriate - either if we were explicitly asked (using -B),  
                // or if the desired FileSink buffer size happens to be larger than the current OS socket buffer size.  
                // (The latter case is a heuristic, on the assumption that if the user asked for a large FileSink buffer size,  
                // then the input data rate may be large enough to justify increasing the OS socket buffer size also.)  
                int socketNum = subsession->rtpSource()->RTPgs()->socketNum();  
                unsigned curBufferSize = getReceiveBufferSize(*env, socketNum);  
                unsigned newBufferSize = setReceiveBufferTo(*env, socketNum, 100000);  
  
            }  
        }  
    }  
    //由RTSPClient对象向服务器发送SETUP消息并接受回应  
    iter.reset();  
    while ((subsession = iter.next()) != NULL) {  
        if (subsession->clientPortNum() == 0) continue; // port # was not set  
        if (!rtspClient->setupMediaSubsession(*subsession)) {  
            *env << "Failed to setup /"" << subsession->mediumName()  
                << "/" << subsession->codecName()  
                << "/" subsession: " << env->getResultMsg() << "/n";  
        } else {  
            *env << "Setup /"" << subsession->mediumName()  
                << "/" << subsession->codecName()  
                << "/" subsession (client ports " << subsession->clientPortNum()  
                << "-" << subsession->clientPortNum()+1 << ")/n";  
        }  
        if (subsession->rtpSource() != NULL) {  
            // Because we're saving the incoming data, rather than playing  
            // it in real time, allow an especially large time threshold  
            // (1 second) for reordering misordered incoming packets:  
            unsigned const thresh = 1000000; // 1 second  
            subsession->rtpSource()->setPacketReorderingThresholdTime(thresh);  
        }  
    }  
    iter.reset();  
    while ((subsession = iter.next()) != NULL) {  
        if (subsession->readSource() == NULL) continue; // was not initiated  
        char outFileName[1000];  
        static unsigned streamCounter = 0;  
        snprintf(outFileName, sizeof outFileName, "%s-%s-%d",  
            subsession->mediumName(),  
            subsession->codecName(), ++streamCounter);  
        FileSink* fileSink;  
        if (strcmp(subsession->mediumName(), "audio") == 0 &&  
            (strcmp(subsession->codecName(), "AMR") == 0 ||  
            strcmp(subsession->codecName(), "AMR-WB") == 0)) {  
                // For AMR audio streams, we use a special sink that inserts AMR frame hdrs:  
                fileSink = AMRAudioFileSink::createNew(*env, outFileName);  
        } else if (strcmp(subsession->mediumName(), "video") == 0 &&  
            (strcmp(subsession->codecName(), "H264") == 0)) {  
                // For H.264 video stream, we use a special sink that insert start_codes:  
                unsigned int num=0;  
                SPropRecord * sps=parseSPropParameterSets(subsession->fmtp_spropparametersets(),num);  
                fileSink = H264VideoFileSink::createNew(*env, outFileName,100000);  
                struct timeval tv={0,0};  
                unsigned char start_code[4] = {0x00, 0x00, 0x00, 0x01};  
                fileSink->addData(start_code, 4, tv);  
                fileSink->addData(sps[0].sPropBytes,sps[0].sPropLength,tv);  
                fileSink->addData(start_code, 4, tv);  
                fileSink->addData(sps[1].sPropBytes,sps[1].sPropLength,tv);  
                delete[] sps;  
        } else {  
            // Normal case:  
            fileSink = FileSink::createNew(*env, outFileName);  
        }  
        subsession->sink = fileSink;  
        subsession->sink->startPlaying(*(subsession->readSource()),NULL,NULL);  
    }  
    rtspClient->playMediaSession(*session, 0.0f, 0.0f, (float)1.0);  
    env->taskScheduler().doEventLoop(); // does not return  
    return 0; // only to prevent compiler warning  
}