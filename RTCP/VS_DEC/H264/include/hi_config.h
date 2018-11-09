/******************************************************************************

  Copyright (C), 2007-2017, Hisilicon Tech. Co., Ltd.
  ******************************************************************************
  File Name     : hi_config.h
  Version       : Initial Draft
  Author        : Hisilicon multimedia
  Created       : 2007/06/28
  Description   : for system  compatibility
  History       :
  1.Date        : 2007/06/28
  Author        : y39262
  Modification  : Created file
******************************************************************************/


#ifndef __HI_CONFIG__
#define __HI_CONFIG__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <malloc.h>
#include <windows.h>
#include <process.h> // compile with: /MT

/* if using dynamic linking library, HI_DLL_CALL should be defined */
//#define HI_DLL_CALL

#ifdef HI_DLL_CALL
#define HI_DLLEXPORT __declspec( dllexport )
#else 
#define HI_DLLEXPORT 
#endif


#define CONSTVOID const void
typedef unsigned char   HI_U8;
typedef unsigned char   HI_UCHAR;
typedef unsigned short  HI_U16;
typedef unsigned long   HI_U32;
typedef signed char     HI_S8;
typedef signed short    HI_S16;
typedef signed long     HI_S32;
typedef char            HI_CHAR;
typedef char*           HI_PCHAR;
typedef float           HI_FLOAT;
typedef double          HI_DOUBLE;
typedef void            HI_VOID;
typedef enum {
    HI_FALSE = 0,
    HI_TRUE  = 1,
} HI_BOOL;

#define HI_NULL                 0L
#define HI_NULL_PTR             HI_NULL

#define HI_SUCCESS              0
#define HI_FAILURE              (-1)

#define  HI_LITTLE_ENDIAN       1234      
#define  HI_BIG_ENDIAN          4321      
#define  HI_DECODER_SLEEP_TIME  60000


#ifdef _MSC_VER
typedef __int64                 HI_S64;
typedef unsigned __int64        HI_U64;
#endif

#if defined __INTEL_COMPILER || defined __GNUC__
typedef long long               HI_S64;
typedef unsigned long long      HI_U64;
#endif


#endif __HI_CONFIG__



