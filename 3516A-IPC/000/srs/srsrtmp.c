#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "srs_librtmp.h"
#include "srsrtmp.h"
#include "bufpool2.h"

static char spsdata[50];
static char ppsdata[50];
static int  spslen = 0;
static int  ppslen = 0;

void set_srssps(char *data, int size)
{
  memcpy(spsdata+4, data, size);
  spsdata[0] = 0;
  spsdata[1] = 0;
  spsdata[2] = 0;
  spsdata[3] = 1;
  spslen = size+4;
}
void set_srspps(char *data, int size)
{
  memcpy(ppsdata, data, size);
  ppsdata[0] = 0;
  ppsdata[1] = 0;
  ppsdata[2] = 0;
  ppsdata[3] = 1;
  ppslen = size+4;
}

int srs_streamrun = 0;
void *SrsStreamMain(void* p)
{
  printf("========SRS=======\n");
  //srs_rtmp_t rtmp = srs_rtmp_create("rtmp://192.168.1.7:1935/hls");
  //srs_rtmp_t rtmp = srs_rtmp_create("rtmp://192.168.1.8:1935/live/720p");
  srs_rtmp_t rtmp = srs_rtmp_create("rtmp://148.70.17.190:1935/live/720p");
  
  if(srs_rtmp_handshake(rtmp) != 0) {
	printf("simple handshake failed.\n");
	goto rtmp_destroy;
  }
  printf("simple handshake success.\n");

  if(srs_rtmp_connect_app(rtmp) != 0) {
	printf("connect vhost/app failed.\n");
	goto rtmp_destroy;
  }
  printf("connect vhost/app success.\n");

  if(srs_rtmp_publish_stream(rtmp) != 0) {
	printf("publish stream failed.\n");
	goto rtmp_destroy;
  }
  int dts = 0;  int pts = 0;
  
  //*****************************//
  char stream[500*1024];
  BufData data;
  data.bptr = stream;
  data.len  = 0;
  
  char srsdata[500*1024];
  int  srslen = 0;
  
  srs_streamrun = 1;
  while(srs_streamrun)
  {
    if(PullBuf(&data) && data.len>0)
    {
	  int NalType = data.bptr[0]&0x1f;
	  
	  if(NalType==7 && dts>1000)
	  {
		continue;
	  }
	  if(NalType==8 && dts>1000)
	  {
		continue;
	  }
	  
	  srslen = 0;
	  srsdata[srslen+0] = 0;
	  srsdata[srslen+1] = 0;
	  srsdata[srslen+2] = 0;
	  srsdata[srslen+3] = 1;
	  srslen += 4;
	  
	  memcpy(srsdata + srslen,  data.bptr, data.len);
	  srslen += data.len;
	  printf("===========%d   %d=========\n", srslen-4, NalType);
	  
	  int ret = srs_h264_write_raw_frames(rtmp, srsdata, srslen, dts, pts);
	  if (ret != 0) 
	  {
	    if(srs_h264_is_dvbsp_error(ret)) {
	    printf("ignore drop video error, code=%d\n", ret);
	    }else if (srs_h264_is_duplicated_sps_error(ret)) {
	    printf("ignore duplicated sps, code=%d\n", ret);
	    }else if (srs_h264_is_duplicated_pps_error(ret)) {
	    printf("ignore duplicated pps, code=%d\n", ret);
	    }else {
	    printf("send h264 raw data failed. ret=%d\n", ret);
	    goto rtmp_destroy;
	    }
	  }
	  
	  if(NalType !=7 && NalType !=8)
	      dts += 1000/30;
	  pts = dts;
    }
  }

rtmp_destroy:
  srs_rtmp_destroy(rtmp);
}

pthread_t pt_srsrtmp;
int srsrtmp_run()
{
  pthread_create(&pt_srsrtmp, 0, SrsStreamMain, NULL);
}

int srsrtmp_exit()
{
  srs_streamrun = 0;
  usleep(200*1000);
}