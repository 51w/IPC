#ifdef __cplusplus  
extern "C"
{
#endif 

#include <libavcodec/avcodec.h>

typedef struct _YuvData
{
	int w;
	int h;
	unsigned char *data[3];
}YuvData;

int InitDecoder(int codec_id);

void UninitDecoder(void);

int DecodeFrame(unsigned char * framedata, int framelen, YuvData* yuv);

#ifdef __cplusplus  
};
#endif