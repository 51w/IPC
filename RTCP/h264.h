#include <stdio.h>  
#include <stdlib.h>   
#include <string.h>  
#pragma comment( lib, "ws2_32.lib" )    
  
#define PACKET_BUFFER_END      (unsigned int)0x00000000  
  
#define MAX_RTP_PKT_LENGTH     15000  
  
#define H264                    96  
  
/****************************************************************** 
RTP_FIXED_HEADER 
0                   1                   2                   3 
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ 
|V=2|P|X|  CC   |M|     PT      |       sequence number         | 
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ 
|                           timestamp                           | 
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ 
|           synchronization source (SSRC) identifier            | 
+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+ 
|            contributing source (CSRC) identifiers             | 
|                             ....                              | 
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ 
 
******************************************************************/  
typedef struct   
{  
    /* byte 0 */  
    unsigned char csrc_len:4; /* CC expect 0 */  
    unsigned char extension:1;/* X  expect 1, see RTP_OP below */  
    unsigned char padding:1;  /* P  expect 0 */  
    unsigned char version:2;  /* V  expect 2 */  
    /* byte 1 */  
    unsigned char payload:7; /* PT  RTP_PAYLOAD_RTSP */  
    unsigned char marker:1;  /* M   expect 1 */  
    /* byte 2,3 */  
    unsigned short seq_no;   /*sequence number*/  
    /* byte 4-7 */  
    unsigned int timestamp;
    /* byte 8-11 */  
    unsigned int ssrc; /* stream number is used here. */  
} __attribute__((packed)) RTP_FIXED_HEADER;/*12 bytes*/
//取消结构在编译过程中的优化对齐，按照实际占用字节数进行对齐 
  
/****************************************************************** 
NALU_HEADER 
+---------------+ 
|0|1|2|3|4|5|6|7| 
+-+-+-+-+-+-+-+-+ 
|F|NRI|  Type   | 
+---------------+ 
******************************************************************/  
typedef struct {  
    //byte 0  
    unsigned char TYPE:5;  
    unsigned char NRI:2;  
    unsigned char F:1;  
}  __attribute__((packed)) NALU_HEADER; /* 1 byte */  
  
  
/****************************************************************** 
FU_INDICATOR 
+---------------+ 
|0|1|2|3|4|5|6|7| 
+-+-+-+-+-+-+-+-+ 
|F|NRI|  Type   | 
+---------------+ 
******************************************************************/  
typedef struct {  
    //byte 0  
    unsigned char TYPE:5;  
    unsigned char NRI:2;   
    unsigned char F:1;           
}  __attribute__((packed)) FU_INDICATOR; /*1 byte */  
  
  
/****************************************************************** 
FU_HEADER 
+---------------+ 
|0|1|2|3|4|5|6|7| 
+-+-+-+-+-+-+-+-+ 
|S|E|R|  Type   | 
+---------------+ 
******************************************************************/  
typedef struct {  
    //byte 0  
    unsigned char TYPE:5;  
    unsigned char R:1;  
    unsigned char E:1;  
    unsigned char S:1;      
}  __attribute__((packed)) FU_HEADER; /* 1 byte */
