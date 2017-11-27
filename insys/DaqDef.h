////////////////////////////////////////////////////////////////
//      DAQDEF.H - DAQ definitions
//      Copyright (c) 1998, Instrumental Systems,Corp.
////////////////////////////////////////////////////////////////

#ifndef _DAQDEF_H_
#define _DAQDEF_H_

#include <stdlib.h>

#define MAX_DAQ_DEV_NUM	32

#define DAQ_NUMDEVS		MAX_DAQ_DEV_NUM
typedef long DAQ_DEVICE;

#ifdef __cplusplus
    extern "C" {
#endif

/* Было _stdcall  - наверно для Visual Studio C++? Либо ошибка.*/        
int __stdcall DAQ_close (int   hDaq);
int __stdcall DAQ_ioctl (int   hDaq,
						long  ioctlFxn, 
						void* ioctlPars); 
int __stdcall DAQ_open  (LPCTSTR DriverName, void* pDrvPars);
int __stdcall DAQ_read  (int hDaq,
						void* Buffer, 
						ULONG BufSize, 
						long  msTimeout);
int __stdcall DAQ_write (int	   hDaq,
						void*  Buffer, 
						ULONG  BufSize, 
						long   msTimeout); 

LPCTSTR __stdcall DAQ_GetErrorMessage (int hDaq, int ErrorIndex); 

#ifdef __cplusplus
    }
#endif

typedef enum _DAQ_IOCTL_FXNS {
	DAQ_ioctlSETRATE			= 0,
	DAQ_ioctlGETRATE			= 1,
	DAQ_ioctlSETGAIN			= 2,
	DAQ_ioctlGETGAIN			= 3,
	DAQ_ioctlSETNUMCHANS		= 4,
	DAQ_ioctlGETNUMCHANS		= 5,
	DAQ_ioctlSETREGDATA			= 6,
	DAQ_ioctlGETREGDATA			= 7,
	DAQ_ioctlREADREGDATA		= 8,
	DAQ_ioctlRESTOREREGDATA		= 9,  // new Dorokhin 15.04.2003
	DAQ_ioctlGETERRCNT			= 10, // not usage
	DAQ_ioctlGETERRMSG			= 11, // not usage
	DAQ_ioctlGETXFERTIME		= 12, // not usage
	DAQ_ioctlAREDATALOST		= 13,
	DAQ_ioctlGETDEVCAPS			= 14, // not usage
	DAQ_ioctlREAD_MEM_REQUEST	= 20,
	DAQ_ioctlASYNC_TRANSFER		= 21,
	DAQ_ioctlREAD_BUFISCOMPLETE	= 22,
	DAQ_ioctlREAD_GETIOSTATE	= 23,
	DAQ_ioctlREAD_ABORTIO		= 24,
	DAQ_ioctlSYSTEM_BUFFER_COPY	= 25,
	DAQ_ioctlASYNC_FILE			= 26, // not usage
	DAQ_ioctlREAD_MEM_RELEASE	= 27,
	DAQ_ioctlREAD_ISCOMPLETE	= 28,
	DAQ_ioctlREAD_COMPLETE		= 29,
	DAQ_ioctlFILE_IO			= 30,
	DAQ_ioctlFILEIO_ISCOMPLETE	= 31,
	DAQ_ioctlFILEIO_ABORT		= 32,
	DAQ_ioctlFILEIO_GETSTATUS	= 33,
	DAQ_ioctlFILEIO_GETNUMBUFS	= 34,
	DAQ_ioctlFILE_IO_BUFFERING	= 35, // new 25.03.2004 Tchurzin
	DAQ_ioctlBUSMASTERING		= 40,
	DAQ_ioctlSETMASTERSLAVE		= 41,
	DAQ_ioctlGETMASTERSLAVE		= 42,
	DAQ_ioctlWRITE_MEM_REQUEST	= 61, // new
	DAQ_ioctlWRITE_BUFISCOMPLETE= 62, // new
	DAQ_ioctlWRITE_GETIOSTATE	= 63, // new
	DAQ_ioctlWRITE_ABORTIO		= 64, // new
	DAQ_ioctlWRITE_MEM_RELEASE	= 67, // new
	DAQ_ioctlWRITE_ISCOMPLETE	= 68, // new
	DAQ_ioctlWRITE_COMPLETE		= 69, // new
	DAQ_ioctlWALKERRORTABLE		= 90, // not usage
	DAQ_ioctlDRVSPEC			= 100 // not usage
} DAQ_IOCTL_FXNS;

#pragma pack(push,1)

typedef struct TBLENTRY {
  PVOID Addr;
  ULONG Size;
} TBLENTRY, *PTBLENTRY;

typedef struct DAQ_ASYNCREQDATA {
  int BufCnt;
  TBLENTRY Tbl[1];
} DAQ_ASYNCREQDATA, *PDAQ_ASYNCREQDATA;

typedef struct DAQ_FLTPTPAR {
  float Value;
  long  Chan;
} DAQ_FLTPTPAR, *PDAQ_FLTPTPAR;

typedef struct DAQ_IOCTLREG {
  long  RegId;
  ULONG Mask;
} DAQ_IOCTLREG, *PDAQ_IOCTLREG;

typedef struct DAQ_IOCTLFILEIO {
  long  Dir;
  long  Offset;
  ULONG BufSize;
  ULONG NumBufs;
  ULONG msTimeout;
  char  FileName[_MAX_PATH];
} DAQ_IOCTLFILEIO, *PDAQ_IOCTLFILEIO;

typedef struct DAQ_ASYNCXFERSTATE {
  long  CurBuf;
  ULONG CurSize;
  ULONG InitCnt;
} DAQ_ASYNCXFERSTATE, *PDAQ_ASYNCXFERSTATE;

typedef struct DAQ_ASYNCCOMPLETE {
	ULONG msTimeout;
	DAQ_ASYNCXFERSTATE State;
} DAQ_ASYNCCOMPLETE, *PDAQ_ASYNCCOMPLETE;

typedef struct DAQ_ASYNCXFERDATA {
  long UseIrq;
  long Dir;
  long AutoInit;
} DAQ_ASYNCXFERDATA, *PDAQ_ASYNCXFERDATA;


typedef enum DAQ_DATA_DIRECTION { 
	DIR_FROM_DEVICE_TO_MEMORY = 1, 
	DIR_FROM_MEMORY_TO_DEVICE 
} DAQ_DATA_DIRECTION;

typedef struct DAQ_SYSTEM_BUFFER_COPY {
    PVOID              pUserBuf;
    PVOID              pSysBuf;
    ULONG              Size;
    DAQ_DATA_DIRECTION Dir;
} DAQ_SYSTEM_BUFFER_COPY, *PDAQ_SYSTEM_BUFFER_COPY;

typedef enum _DAQ_ERROR_STATUS {  
    DAQ_errOK,
    DAQ_errINCORRECT_HANDLE = 0xE0000000UL,
    DAQ_errCLOSE,
    DAQ_errILLEGAL
}DAQ_ERROR_STATUS;

#pragma pack(pop)


#define STATUS_CUSTOMER_CODE_FLAG    0x20000000L

//
// MessageId: STATUS_CUSTOM_INSUFFICIENT_BUFFERS
//
// MessageText:
//
//  Insufficient custom buffers exist to complete the API.
//
#define STATUS_CUSTOM_INSUFFICIENT_BUFFERS   \
			((NTSTATUS)((STATUS_SEVERITY_WARNING<<30) | STATUS_CUSTOMER_CODE_FLAG | 0x01))     

#endif //_DAQDEF_H_
