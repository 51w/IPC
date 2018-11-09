/******************************************************************************

  Copyright (C), 2007-2017, Hisilicon Tech. Co., Ltd.
  ******************************************************************************
  File Name     : hi_h264api.h
  Version       : Initial Draft
  Author        : Hisilicon multimedia
  Created       : 2007/07/31
  Description   :
  History       :
  1.Date        : 2007/07/31
  Author        : d59179
  Modification  : Created file
******************************************************************************/

#ifndef __HI_H264API__
#define __HI_H264API__


#define  HI_H264DEC_OK               0
#define  HI_H264DEC_NEED_MORE_BITS  -1
#define  HI_H264DEC_NO_PICTURE      -2
#define  HI_H264DEC_ERR_HANDLE      -3

typedef void* HI_HDL;

/**********************************************************************
* This data structure describes the build information
* and function set of decoder library.
***********************************************************************/
typedef struct hiH264_LIBINFO_S
{
    HI_U32  uMajor;              //Major Version
    HI_U32  uMinor;              //Minor Version
    HI_U32  uRelease;            //Release Version 
    HI_U32  uBuild;              //Build Version
    const HI_CHAR*   sVersion;   //Version information
    const HI_CHAR*   sCopyRight; //CopyRight information
    HI_U32  uFunctionSet;        //Function set of the decode library  
    HI_U32  uPictureFormat;      //Picture format 0x00:YUV420 0x01:YUV422 0x02:YUV444
    HI_U32  uStreamInType;       //Input stream type
    HI_U32  uPicWidth;           //The max Width of picture in pixel
    HI_U32  uPicHeight;          //The max Height of picture in pixel  
    HI_U32  uBufNum;             //Max reference frame num
    HI_U32  uReserved;           //Reserved
} H264_LIBINFO_S;

/************************************************************
 * This data structure describes the information of userdata.
************************************************************/
typedef struct hiH264_USERDATA_S
{
    HI_U32             uUserDataType;   //Type of userdata
    HI_U32             uUserDataSize;   //Length of userdata in byte
    HI_UCHAR*          pData;           //Buffer contains userdata stuff
    struct hiH264_USERDATA_S* pNext;    //Pointer to next userdata
} H264_USERDATA_S;


/***************************************************************************
 * This data structure describes the attributes during the decoding process.
****************************************************************************/
typedef struct hiH264_DEC_ATTR_S
{
    HI_U32  uPictureFormat;       //Decoded output picture format 0x00:YUV420 0x01:YUV422 0x02:YUV444
    HI_U32  uStreamInType;        //Input stream type
    HI_U32  uPicWidthInMB;        //The width of picture in MB
    HI_U32  uPicHeightInMB;       //The height of picture in MB
    HI_U32  uBufNum;              //Max reference frame num 
    HI_U32  uWorkMode;            //Decoder working mode 
    H264_USERDATA_S  *pUserData;  //Buffer contains userdata stuff
    HI_U32  uReserved;
} H264_DEC_ATTR_S;

/**************************************************************
* This data structure describes the output information for each frame.
***************************************************************/
typedef struct hiH264_DEC_OUTPUT_INFO_S
{
	HI_U32 uPicBytes;            //total bytes of one frame
	HI_U32 uI4MbNum;             //number of I4x4 macroblocks in one frame
	HI_U32 uI8MbNum;             //number of I8x8 macroblocks in one frame
	HI_U32 uI16MbNum;            //number of I16x16 macroblocks in one frame
	HI_U32 uP16MbNum;            //number of P16x16 macroblocks in one frame
	HI_U32 uP16x8MbNum;          //number of P16x8 macroblocks in one frame
	HI_U32 uP8x16MbNum;          //number of P8x16 macroblocks in one frame
	HI_U32 uP8MbNum;             //number of P8x8 macroblocks in one frame
	HI_U32 uPskipMbNum;          //number of PSkip macroblocks in one frame
	HI_U32 uIpcmMbNum;           //number of IPCM macroblocks in one frame
} H264_DEC_OUTPUT_INFO_S;


/**************************************************************
* This data structure describes the attribute of output picture 
* of the decoding process for each frame.
***************************************************************/
typedef struct hiH264_DEC_FRAME_S
{
    HI_U8*  pY;                   //Y plane base address of the picture
    HI_U8*  pU;                   //U plane base address of the picture
    HI_U8*  pV;                   //V plane base address of the picture
    HI_U32  uWidth;               //The width of output picture in pixel
    HI_U32  uHeight;              //The height of output picture in pixel
    HI_U32  uYStride;             //Luma plane stride in pixel
    HI_U32  uUVStride;            //Chroma plane stride in pixel
    HI_U32  uCroppingLeftOffset;  //Crop information in pixel
    HI_U32  uCroppingRightOffset; 
    HI_U32  uCroppingTopOffset;   
    HI_U32  uCroppingBottomOffset;
    HI_U32  uDpbIdx;              //The index of dpb
    HI_U32  uPicFlag;             //0: Frame; 1: Top filed; 2: Bottom field  
    HI_U32  bError;               //0: picture is correct; 1: picture is corrupted
    HI_U32  bIntra;               //0: intra picture; 1:inter picture
    HI_U64  ullPTS;               //Time stamp
    HI_U32  uPictureID;           //The sequence ID of this output picture decoded
    HI_U32  uReserved;            //Reserved for future
    H264_USERDATA_S *pUserData;   //Pointer to the first userdata
	H264_DEC_OUTPUT_INFO_S *pFrameInfo; //Pointer to the output information of one frame
} H264_DEC_FRAME_S;


#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
* Function Name  : Hi264DecImageEnhance
* Description    : image enhance
* Parameters     : hDec         : decoder handle created by HI_H264DEC_Create
*                : pDecFrame    : the decoded Parameters of the picture frame 
*                                 (referring to H264_DEC_FRAME_S ).
*                : uEnhanceCoeff: image enhancement coefficient 
* Return Type    : if success, return HI_H264DEC_OK; 
*                  otherwise,  return HI_H264DEC_ERR_HANDLE. 
* Last Modified  : 2008-5-6
*********************************************************************/
HI_S32 HI_DLLEXPORT Hi264DecImageEnhance(HI_HDL hDec, H264_DEC_FRAME_S *pDecFrame, HI_U32 uEnhanceCoeff);

/*********************************************************************
* Function Name  : Hi264DecCreate
* Description    : Create and initialize H.264 decoder handle
* Parameters     : pDecAttr:   a pointer referring to H264_DEC_ATTR_S 
*                              which contain the needed  attributes to 
*                              initialize the decoder
* Return Type    : if success, return a decoder handle; 
*                  otherwise,  return NULL. 
* Last Modified  : 2007-8-27
*********************************************************************/
HI_HDL HI_DLLEXPORT Hi264DecCreate( H264_DEC_ATTR_S *pDecAttr );

/***************************************************************** 
* Function Name  : Hi264DecDestroy
* Description    : destroy H.264 decoder handle
* Parameters     : hDec:  decoder handle created by Hi264DecCreate
* Return Type    : none
* Last Modified  : 2007-8-27
*****************************************************************/
void HI_DLLEXPORT Hi264DecDestroy( HI_HDL hDec );

/**************************************************************************** 
* Function Name : Hi264DecGetInfo
* Description   : get the build and owner infomation of this function library
* Parameters    : H264_LIBINFO_S *pLibInfo
* Return Type   : if success, return 0; 
*                 otherwise,  return -1.
* Last Modified : 2007-8-27
****************************************************************************/
HI_S32 HI_DLLEXPORT Hi264DecGetInfo( H264_LIBINFO_S *pLibInfo );

/********************************************************************************* 
* Function Name  : Hi264DecFrame
* Description    : input stream and decode 
* Parameters     : hDec       : decoder handle created by HI_H264DEC_Create
*                : pStream    : stream buffer 
*                : iStreamLen : stream length in byte
*                : ullPTS     : time stamp
*                : pDecFrame  : denoting whether there is a picture frame to display, 
*                               and the decoded Parameters of the picture frame 
*                               (referring to H264_DEC_FRAME_S ).
*                : uFlags     : working mode  
* Return Type    : if success : return HI_H264DEC_OK; 
*                  otherwise  : return the corresponding error code.
* Last Modified  : 2007-8-27
**********************************************************************************/
HI_S32 HI_DLLEXPORT Hi264DecFrame(
    HI_HDL  hDec,
    HI_U8*  pStream,
    HI_U32  iStreamLen,
    HI_U64  ullPTS,
    H264_DEC_FRAME_S *pDecFrame,
    HI_U32  uFlags );

/*********************************************************************************
* Function Name  : Hi264DecAU
* Description    : input an au-stream and decode one au 
* Parameters     : hDec       : decoder handle created by HI_H264DEC_Create
*                : pStream    : stream buffer
*                : iStreamLen : stream length in byte
*                : ullPTS     : time stamp
*                : pDecFrame  : denoting whether there is a picture frame to display,
*                               and the decoded Parameters of the picture frame
*                               (referring to H264_DEC_FRAME_S ).
*                : uFlags     : working mode, invalid 
* Return Type    : if success : return HI_H264DEC_OK;
*                  otherwise  : return the corresponding error code.
* Last Modified  : 2008-1-7
**********************************************************************************/
HI_S32 HI_DLLEXPORT Hi264DecAU(
  HI_HDL hDec,
  HI_U8 *pStream,
  HI_U32 iStreamLen,
  HI_U64 ullPTS,
  H264_DEC_FRAME_S *pDecFrame,
  HI_U32 uFlags );


#ifdef __cplusplus
}
#endif
#endif __HI_H264API__
