#include "H264VideoStreamSource.hh"
#include "bufpool.h"

H264VideoStreamSource*
H264VideoStreamSource::createNew(UsageEnvironment& env)
{
	H264VideoStreamSource* newSource = new H264VideoStreamSource(env);

	return newSource;
}

H264VideoStreamSource::H264VideoStreamSource(UsageEnvironment& env)
	: FramedSource(env)
{//
}

H264VideoStreamSource::~H264VideoStreamSource() 
{
}

void H264VideoStreamSource::doGetNextFrame() 
{
	unsigned int stream_len = 0;
	
	BufData bufdata;
	if( PopBuf(&bufdata, 0) )
	{
		stream_len = bufdata.len;
		if (stream_len > fMaxSize) {
			printf("[1]  drop stream: length=%u, fMaxSize=%d\n", stream_len, fMaxSize);
			stream_len = 0;
			goto out;
		}

		memcpy((void *)fTo, (void *)(bufdata.bptr), stream_len);

		gettimeofday(&fPresentationTime, NULL);
		
		//if(bufdata.naltype != 7 && bufdata.naltype != 8 && bufdata.naltype != 6)
		//usleep(20*1000);
	}
out:
	fFrameSize = stream_len;
	FramedSource::afterGetting(this);
}