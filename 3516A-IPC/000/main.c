#include <stdio.h>
#include <unistd.h>

#include "stream.h"
#include "librtspserver.hh"
#include "rtmpstream.h"
extern int autonet();
extern int BroadCast_UDP();

int isrtmp()
{
  FILE* fp = fopen("/home/isrtmp.conf", "rb");
  int ZZZ = 0;
  fscanf(fp, "%d", &ZZZ);
  printf("Isrtmp [%d]\n", ZZZ);
  fclose(fp);
  
  return ZZZ;
}

int main()
{
  printf("======================\n");
  IPC_main();
  usleep(2000*1000);

  int net2 = autonet();
  if(net2){
    printf(">>>Wifi > Internet\n");
    BroadCast_UDP();
    
    if(isrtmp())
    {
      set_bitrate(600);
      //rtmpstream_run();   
      srsrtmp_run();
    }
    else{
      set_bitrate(5000);
      RtspServer();      
    }
  }
  else{
    printf(">>>Wifi > AP\n");
    BroadCast_UDP();
    
    set_bitrate(1000);
    RtspServer();
  }	

  while(1)
  {
  usleep(2000*1000);
  }

  //RtspServerExit();
  //rtmpstream_exit();
  srsrtmp_exit();

  IPC_exit();
  printf("======================\n");
}