#ifdef __cplusplus  
extern "C"
{
#endif

#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include "dec264.h"

AVCodec *codec = NULL;
AVCodecParserContext *parser;
AVCodecContext *codec_ctx = NULL;
AVFrame *frame = NULL;
AVPacket *pkt;
int codecInited = 0;

AVPicture  pAVPicture;

int InitDecoder(int codec_id)
{
	if (codecInited != 0)
	{
		printf("decoder inited fail\n");
		return -1;
	}

	pkt = av_packet_alloc();
	if (!pkt)
	{
		printf("av_packet_alloc fail\n");
		return -1;
	}
	
	codec = avcodec_find_decoder(codec_id);
	if (!codec)
	{
		printf("avcodec_find_decoder fail\n");
		return -1;
	}
	//printf("==%s\n", codec->long_name);

	parser = av_parser_init(codec->id);
	if (!parser) {
		fprintf(stderr, "parser not found\n");
		return -1;
	}

	codec_ctx = avcodec_alloc_context3(codec);
	if (!codec_ctx)
	{
		printf("avcodec_alloc_context3 fail\n");
		return -1;
	}

	if (avcodec_open2(codec_ctx, codec, NULL) < 0)
	{
		printf("avcodec_open2 fail\n");
		return -1;
	}

	frame = av_frame_alloc();
	if (!frame)
	{
		printf("av_frame_alloc fail\n");
		return -1;
	}

	codecInited = 1;
	return 0;
}

void UninitDecoder(void)
{
	if (codecInited)
	{
		av_parser_close(parser);
		avcodec_free_context(&codec_ctx);
		
		av_frame_free(&frame);
		av_packet_free(&pkt);
		codecInited = 0;
	}
}

int decode(AVCodecContext *pdec_ctx, AVFrame *pframe, AVPacket *ppkt, YuvData *yuv)
{
	int ret = avcodec_send_packet(pdec_ctx, ppkt);
	if (ret < 0) {
		fprintf(stderr, "Error sending a packet for decoding\n");
		return -1;
	}

	while (ret >= 0)
	{
		ret = avcodec_receive_frame(pdec_ctx, pframe);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
		{
			//fprintf(stderr, "EAGAIN or AVERROR_EOF\n");
			return -1;
		}
		else if (ret < 0)
		{
			fprintf(stderr, "Error during decoding\n");
			return -1;
		}
		//printf("%d %d %d\n", pframe->linesize[0], pframe->width, pframe->height);
		//printf("saving frame %3d\n", pdec_ctx->frame_number);

		yuv->data[0] = pframe->data[0];
		yuv->data[1] = pframe->data[1];
		yuv->data[2] = pframe->data[2];
		yuv->w = pframe->width;
		yuv->h = pframe->height;

		return 0;
	}
	return 0;
}

int DecodeFrame(unsigned char * framedata, int framelen, YuvData *yuv)
{
	int success = -1;

	while (framelen > 0)
	{
		int ret = av_parser_parse2(parser, codec_ctx, &pkt->data, &pkt->size, framedata, framelen, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
		if (ret < 0)
		{
			fprintf(stderr, "Error while parsing\n");
			return -1;
		}
		framedata += ret;
		framelen  -= ret;
		
		if (pkt->size)
		success = decode(codec_ctx, frame, pkt, yuv);
	}

	return success;
}

#ifdef __cplusplus  
};
#endif