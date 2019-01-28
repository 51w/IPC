#ifdef __cplusplus
#if __cplusplus
extern "C" 
{
#endif
#endif

int rtmp_connect(char* url);
int rtmp_close();

void setsps(char *data, int size);
void setpps(char *data, int size);

int rtmpstream_run();
int rtmpstream_exit();

int SendH264Packet(char *data, unsigned int size, int bIsKeyFrame, unsigned int nTimeStamp);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif