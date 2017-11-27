/*
 ****************** File ambpcidf.h *************************
 *
 *  Definitions of user application interface
 *	structures and constants
 *  for AMBPCI and AMBPCM Data Acquisition Carrier Board.
 *
 ************************************************************
*/

#ifndef _AMBPCIDF_H
#define _AMBPCIDF_H

#define AMB_MAXREFS 2

#define AMCC_DEFDID		0x4750	// DeviceID for AMCC S593x
#define AMBPCI_DEFDID	0x4D50	// DeviceID for AMBPCI
#define DDS9854_DEFDID	0x4D51	// DeviceID for DDS9854
#define AMBPCM_DEFDID	0x4D23	// DeviceID for AMBPCM
#define ADS1648_DEFDID	0x1648	// DeviceID for ADS1616x48
#define ADS2448_DEFDID	0x2048	// DeviceID for ADS224x48
#define VK2_DEFDID		0x564B	// DeviceID for ÂÊ-2

#define AMB_INTGEN0 6.0e7F // Internal clock #1 = 60 MHz
#define AMB_INTGEN1 5.0e7F // Internal clock #2 = 50 MHz
#define AMB_FIFOSIZE 512
#define AMB_idTEST 0
#define AMB_MAXTHR 2.5F  // Reference voltage for THR DACs = 2.5V

typedef enum _AMB_IOCTL_FXNS {
	AMB_ioctlSETADCRATE		= 101,
	AMB_ioctlGETADCRATE		= 102,
	AMB_ioctlSETDACRATE		= 103,
	AMB_ioctlGETDACRATE		= 104,
	AMB_ioctlSETADCMASK		= 105,
	AMB_ioctlGETADCMASK		= 106,
	AMB_ioctlSETPIOMODE		= 107,
	AMB_ioctlGETPIOMODE		= 108,
	AMB_ioctlSETSTARTMODE	= 109,
	AMB_ioctlGETSTARTMODE	= 110,
	AMB_ioctlSETPACKMODE	= 111,
	AMB_ioctlGETPACKMODE	= 112,
	AMB_ioctlSETDACMODE		= 113,
	AMB_ioctlGETDACMODE		= 114,
	AMB_ioctlSETCLKMODE		= 115,
	AMB_ioctlGETCLKMODE		= 116,
	AMB_ioctlGENERATOR		= 117,
	AMB_ioctlDACOFF			= 118,
	AMB_ioctlGENEROFF		= 119,
	AMB_ioctlADCENABLE		= 120,
	AMB_ioctlADCDISABLE		= 121,
	AMB_ioctlPROGINDATA		= 122,
	AMB_ioctlINPORT			= 123,
	AMB_ioctlOUTPORT		= 124,
	AMB_ioctlSETMEMSIZE		= 125,
	AMB_ioctlGETMEMSIZE		= 126,
	AMB_ioctlSETTRIGCNT		= 127,
	AMB_ioctlGETTRIGCNT		= 128,
	AMB_ioctlSETSTARTADDR	= 129,
	AMB_ioctlGETSTARTADDR	= 130,
	AMB_ioctlGETACQSIZE		= 131,
	AMB_ioctlISACQCOMPLETE	= 132,
	AMB_ioctlISPASSMEMEND	= 133,
	AMB_ioctlGETSTATUS		= 134,
	AMB_ioctlSETMEMCFG		= 140,
	AMB_ioctlGETMEMCFG		= 141,
	AMB_ioctlSETREADADDR	= 142,
	AMB_ioctlGETREADADDR	= 143,
	AMB_ioctlSETMEMREADMODE	= 144,
	AMB_ioctlGETMEMREADMODE	= 145,
	AMB_ioctlSETFIFONLYMODE	= 146,
	AMB_ioctlGETFIFONLYMODE	= 147,
	AMB_ioctlMEMREADENBL	= 148,
	AMB_ioctlMEMTESTMODE	= 149,
	AMB_ioctlPROGINDATADW	= 152,	// new v5.55
	AMB_ioctlSETMEMSIZEDW	= 153,	// new v5.55
	AMB_ioctlGETMEMSIZEDW	= 154,	// new v5.55
	AMB_ioctlSETSTADDRDW	= 155,	// new v5.55
	AMB_ioctlGETSTADDRDW	= 156,	// new v5.55
	AMB_ioctlGETACQSIZEDW	= 157,	// new v5.55
	AMB_ioctlSETRDADDRDW	= 158,	// new v5.55
	AMB_ioctlGETRDADDRDW	= 159,	// new v5.55
	AMB_ioctlGETMEMCNTR     = 160,	// new (use by USB)
	AMB_ioctlGETSTARTEVENT	= 161,	// new 07.10.2003 Dorokhin A.
	AMB_ioctlSETTRIGCNTDW	= 162,	// new 08.10.2003 Dorokhin A.
	AMB_ioctlGETTRIGCNTDW	= 163,	// new 08.10.2003 Dorokhin A.
	AMB_ioctlSETADCDIVIDER	= 201,
	AMB_ioctlGETADCDIVIDER	= 202,
	AMB_ioctlSETDACDIVIDER	= 203,
	AMB_ioctlGETDACDIVIDER	= 204,
	AMB_ioctlSETDACSYNCMODE	= 205,
	AMB_ioctlGETDACSYNCMODE	= 206,
	AMB_ioctlWRITETHRDAC	= 207,
	AMB_ioctlWRITEMUXMEM	= 208,
	AMB_ioctlBUSFIFORESET	= 209,
	AMB_ioctlWRITEREGBUF	= 210,
	AMB_ioctlSETOUTDEV		= 211,
	AMB_ioctlSETPCICONFIG	= 301,
	AMB_ioctlGETPCICONFIG	= 302,
	AMB_ioctlWRITEPCICONFIG	= 303,
	AMB_ioctlREADPCICONFIG	= 304,
	AMB_ioctlSETAMBCONFIG	= 305,
	AMB_ioctlGETAMBCONFIG	= 306,
	AMB_ioctlWRITEAMBCONFIG	= 307,
	AMB_ioctlREADAMBCONFIG	= 308,
	AMB_ioctlWRITEADMIDROM	= 309,
	AMB_ioctlREADADMIDROM	= 310,
	AMB_ioctlSETADMID		= 311,
	AMB_ioctlGETADMID		= 312,
	AMB_ioctlWRITEADMID		= 313,
	AMB_ioctlREADADMID		= 314,
	AMB_ioctlILLEGAL
} AMB_IOCTL_FXNS;

typedef enum _AMB_ERROR_STATUS {  
    errOK,  
    errHIGHRATE = 0xE0000100UL,  
    errLOWRATE,  
    errILLCHANID,
    errILLREG,
    errBADPARAM,
    errILLPACK8BITS,
	errILLFXN,
    errPLD_FILEOPEN,
    errPLD_FILEACCESS,
    errPLD_FILEFORMAT,
    errPLD_PROGR,
    errPLD_RECLOST,
    errPLD_ROM,
    errPLD_NOTRDY,
    errDEVICE_ALREADY_OPEN,
    errHIGH_RATE_WR_FILE,
    errALREADY_TRANSFERING,
	errPCM_NOMEM,
	errPCM_NOEQUMEM,
	errPCM_CFGMEMBADPARAM,
    errILLEGAL  
} AMB_ERROR_STATUS;

#pragma pack(push, 1)    

typedef struct AMB_TRIG {
	long On;		// ADC trigger start mode on 
	long Stop;		// ADC stop mode for trigger start 
} AMB_TRIG, *PAMB_TRIG;

typedef struct AMB_START {
	long     Start;   // ADC start mode  
	long     Src;     // Comparator source 
	long     Cmp0Inv; // Comparator 0 pulse inversion 
	long     Cmp1Inv; // Comparator 1 pulse inversion 
	long     Pretrig; // Pretrigger mode on 
	AMB_TRIG Trig;    // Trigger start/stop mode 
	float    Thr[2];  // Thresholds values 
} AMB_START, *PAMB_START;

typedef enum _AmbComparatorSources {
  AMB_cmpEXTSTACLK, // Comp.0 - Ext.Start/Comp.1 - Ext.Clock pins
  AMB_cmpCHAN0CLK,  // Comp.0 - Chan.0/Comp.1 - Ext.Clock pin
  AMB_cmpCHAN1CLK,  // Comp.0 - Chan.1/Comp.1 - Ext.Clock pin
  AMB_cmpCHAN0BOTH  // Chan.0 to both
} AmbComparatorSources;

typedef struct AMB_DAC {
	float Rate;
	long  SyncMode;
} AMB_DAC, *PAMB_DAC;

typedef enum _AmbDacSyncModes {
  AMB_dmASYNCH, // Asynchronous mode
  AMB_dmTIMER,  // Synchronous with timer
  AMB_dmADC,    // Synchronous with ADC
  AMB_dmTIMADC  // Synchronous with timer, synchrostart with ADC 
} AmbDacSyncModes;

typedef struct AMB_CLK {
	long  Src;
	long  Adc;
	float Value;
	long  NumRefs;
	float Refs[AMB_MAXREFS];
} AMB_CLK, *PAMB_CLK;

typedef enum _AmbClockSources {
  AMB_csGEN0, // Generator 0
  AMB_csGEN1, // Generator 1
  AMB_csBUS,  // PCI Bus
  AMB_csEXT   // External clock
} AmbClockSources;

typedef struct AMB_PARAMS {
	float		Rate;
	DWORD		ChanMask;
	long		Pack8Bits;
	AMB_START	Start;
	long		Master;
	AMB_DAC		DacPars;
	AMB_CLK		Clk;
	long		AdcFifoSize;
	long		DacFifoSize;
	long		PioMode;
	long		SubmId;
	long		kbBrdMemSize;
	long		BusMaster;      // Bus Mastering
} AMB_PARAMS, *PAMB_PARAMS;

typedef struct AMB_GENERATOR {
	LPVOID	buf;
	int		size;
	DWORD	DacCycle;
} AMB_GENERATOR, *PAMB_GENERATOR;

typedef struct AMB_PROGINDATA {
	LPVOID	buf;
	DWORD	bSize;
	int		msTimeout;
} AMB_PROGINDATA, *PAMB_PROGINDATA;

typedef struct AMB_PROGINDATADW {
	LPVOID	buf;
	DWORD	dwSize;
	int		msTimeout;
} AMB_PROGINDATADW, *PAMB_PROGINDATADW;

typedef struct AMB_IODATA {
	PVOID	pBuffer;
	ULONG	dwBufferSize;
	ULONG	dwOffset;
} AMB_IODATA, *PAMB_IODATA;

typedef struct AMB_IOBUFREG {
	DWORD	RegId;
	LPVOID	pBuf;
	DWORD	bSize;
} AMB_IOBUFREG, *PAMB_IOBUFREG;

typedef struct HAL_PARAMS {
	WORD	VID;			// PCI Target Vendor ID
	WORD	DID;			// PCI Target Device ID
	WORD	Inst;			// Instanse of device on PCI bus
	WORD	Lat;			// Master latency timer
	DWORD	FifoAdvCtrl;	// Interrupt CSR data
} HAL_PARAMS, *PHAL_PARAMS;

typedef struct AMB_DRVPARS {
  AMB_PARAMS	Pars;           // AMB board parameters
  HAL_PARAMS	HalPars;		// VendorID, DeviceID, InstanceDevice, LatancyTimer
  char			PldFileName[MAX_PATH]; // PLD file name  
} AMB_DRVPARS, *PAMB_DRVPARS;

typedef struct _AMBM_MEMCFG { //DOROKHIN 09.2000
	int Auto;			// 1 - config from SPD
	int	NumSlots;		// Number of Busy Slots (0-4)
	int	RowAddr;		// Number of row addresses (11-14)
	int	ColAddr;		// Number of column addresses (8-13)
	int	NumBanks;		// Number of banks (1,2)
	int	NumBanksChip;	// Number of banks on SDRAM device (2,4)
} AMBM_MEMCFG, *PAMBM_MEMCFG;

typedef struct _PCI_CONFIG {
	WORD	Instance;		// Instanse of device on PCI bus
	WORD	VendorID;		// PCI Vendor ID
	WORD	DeviceID;		// PCI Device ID
	BYTE	RevisionID;		// Version of PCI Device
	BYTE	LatencyTimer;	// Master latency timer
	WORD	BusNum;			// PCI Bus Number
	WORD	DevNum;			// Device Number
	WORD	SlotNum;		// Slot Number
} PCI_CONFIG, *PPCI_CONFIG;

typedef struct _PCI_CFG_EXPROM {
    USHORT  VendorID;   
    USHORT  DeviceID;   
	UCHAR	NotUsed1;
	UCHAR	BusMasterCfg;
	USHORT  NotUsed2;
    UCHAR   RevisionID; 
    UCHAR   ProgIf;     
    UCHAR   SubClass;   
    UCHAR   BaseClass;  
	UCHAR	NotUsed3;
    UCHAR   LatencyTimer;
    UCHAR   HeaderType;  
    UCHAR   BIST;                       // Built in self test
	ULONG	Constant;
    ULONG   BaseAddr[5];
	ULONG	NotUsed4[2];
	ULONG	ExpROMBaseAddr;
	ULONG	NotUsed5[2];
    UCHAR   InterruptLine;
    UCHAR   InterruptPin; 
    UCHAR   MinimumGrant; 
    UCHAR   MaximumLatency; 
} PCI_CFG_EXPROM, *PPCI_CFG_EXPROM;

typedef struct _PAMBOPEN_PARAMS {
  PAMB_PARAMS	pCarPars;       // AMB board parameters
  PHAL_PARAMS	pHalPars;		// VendorID, DeviceID, InstanceDevice, LatancyTimer
  char*			pPldFile;		// PLD file name  
  char*			pDfltPldFile;	// Default PLD file name  
} AMBOPEN_PARAMS, *PAMBOPEN_PARAMS;

#pragma pack(pop)    

// Defaults base module parameters
const AMB_PARAMS AMB_Params = {
  100000.0F,						// ADC Rate in Herts
  0x1,								// Channel mask
  FALSE,                            // 8 Bits packing enable
  { 0,								// Start mode (0)
    0,								// Comparator source (0)
    FALSE,                          // Comparator 0 pulse inversion
    FALSE,                          // Comparator 1 pulse inversion
    FALSE,                          // Pretrigger mode
    { TRUE,                         // Trigger mode
      0},							// Stop mode (0)
    { 0.0F, 0.0F } },               // Threshold values
  TRUE,                             // Master mode
  { 100000.0F,                      // DAC Rate in Herts
    1 },							// DAC synchro-mode (1)
  { 0,								// Clock source (0)   
    FALSE,                          // Sub-module clock source enable 
    AMB_INTGEN0,                    // Clock value in Herts 
    AMB_MAXREFS,                    // Number of reference clocks available 
    { AMB_INTGEN0, AMB_INTGEN1 } }, // Reference clocks  
  AMB_FIFOSIZE,                     // ADC FIFO size in words (512) 
  AMB_FIFOSIZE,                     // DAC FIFO size in words (512) 
  2,								// Port IO mode (2) 
  AMB_idTEST,                       // Sub-module ID (0)
  0,								// Size of memory on the board
  TRUE,								// Bus Mastering enable 
};

const HAL_PARAMS HAL_Params = {
	0,				// PCI Target Vendor ID 
	0,				// PCI Target Device ID 
	0,				// Instanse of device on PCI bus 
	0x20,			// Master latency timer 
	0x20000000		// Interrupt CSR data 
};

#endif // _AMBPCIDF_H

// ****************** End of file ambpcidf.h **********************
