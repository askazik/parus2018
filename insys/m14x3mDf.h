//********************* File m14x3mDf.h *******************
//	ADM214x1M/3M/10M
//	Constants and structures
//	Copyright (c) 2002, Instrumental Systems,Corp.
//	Written by Dorokhin Andrey
//*********************************************************

#ifndef _M14x3MDF_H
#define _M14x3MDF_H

#include "AmbPciDf.h"
#ifdef _USB
#include "AmbUsbDf.h"
#endif // _USB

#pragma pack(1)

//#define M214x3M_MAXRATE 3000000.0
#define M214x3M_MAXMUXCNT 16
#define M214x3M_MAXCHANNEL 2
#define M214x3M_MAXINPUT 8
#define M214x3M_UINPUT 2.5
#define M214x3M_PRECISION 100

typedef struct _M214x3M_GAINPARS {
  float Value;
  int Input;
} M214x3M_GAINPARS, *PM214x3M_GAINPARS;

typedef struct _M214x3M_INPUTPARS {
  float Value;
  int OnOff;
} M214x3M_INPUTPARS, *PM214x3M_INPUTPARS;

typedef struct _M214x3M_INPPARS {
  int MuxCounter;	// Multiplexer Counter 
  M214x3M_GAINPARS Gain[M214x3M_MAXMUXCNT]; // Gain each input
} M214x3M_INPPARS, *PM214x3M_INPPARS;

typedef struct _M214x3M_ZEROSHIFTPARS {
  float Value;
  int Chan;
} M214x3M_ZEROSHIFTPARS, *PM214x3M_ZEROSHIFTPARS;

typedef struct _M214x3M_PARAMS {
  float MaxRate;
  float Rate;
  float ZeroShift[2];
//  int MuxCounter;	// Multiplexer Counter 
//  M214x3M_GAINPARS Gain[M214x3M_MAXMUXCNT]; // Gain each input
  M214x3M_INPUTPARS Gain[M214x3M_MAXMUXCNT]; // Gain each input
} M214x3M_PARAMS, *PM214x3M_PARAMS;

typedef struct _M214x3M_DRVPARS {
  M214x3M_PARAMS Pars;
  AMB_DRVPARS   Carrier;
} M214x3M_DRVPARS, *PM214x3M_DRVPARS;

#pragma pack()

// Codes of Errors
typedef enum _M214x3M_ERROR_STATUS {  
    M214x3M_errOK,  
    M214x3M_errHIGHRATE = 0xE0001000UL,  
    M214x3M_errLOWRATE,  
    M214x3M_errILLGAIN,
	M214x3M_errILLOSV,
    M214x3M_errBADPARAM,
    M214x3M_errILLEGAL  
} M214x3M_ERROR_STATUS;  

// Code Numbers of DAQ_ioctl function
typedef enum _M214x3M_Fxns {
    M214x3M_ioctlSETGAIN		= 1001,
    M214x3M_ioctlGETGAIN		= 1002,
    M214x3M_ioctlSETINPUTOSV	= 1021,
    M214x3M_ioctlGETINPUTOSV	= 1022,
    M214x3M_ioctlSETINP			= 1023,
    M214x3M_ioctlGETINP			= 1024,
    M214x3M_ioctlILLEGAL
} M214x3M_Fxns;

// Defaults submodule parameters
const M214x3M_PARAMS M214x3M_Params = {
	3000000.0, // max rate
	100000.0, // Rate
	0.0, 0.0, // Zero Shift
	{	{ 1., 1}, { 1., 0}, { 1., 0}, { 1., 0},
		{ 1., 0}, { 1., 0}, { 1., 0}, { 1., 0},
		{ 1., 0}, { 1., 0}, { 1., 0}, { 1., 0},
		{ 1., 0}, { 1., 0}, { 1., 0}, { 1., 0} }
};

#endif // _M14x3MDF_H

//**************** End of file m14x3mDf.h ******************
