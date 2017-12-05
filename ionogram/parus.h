// ===========================================================================
// ������������ ���� ��� ������ � �����������.
// ===========================================================================
#ifndef __PARUS_H__
#define __PARUS_H__

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// ������, ��� ��������� � ������ ������������� � �����������
// �� �����������, ����������� � ����������� �������. 
// ����������� ����� � ������������� ������� ����������� ��������
// ������. �������� HDF, CDF �.�.�.
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#include <iomanip>
#include <string>
#include <sstream>
#include <stdexcept>
#include <conio.h>

// ��� ��������� ���������� � ���� ������������ ������� Windows API.
#include <windows.h>

// ������������ ����� ��� ������ � ������� ���.
// ��������� � ������ ���������� ������� daqdrv.dll
#include "daqdef.h"
#pragma comment(lib, "daqdrv.lib") // for Microsoft Visual C++
#include "m14x3mDf.h"

// ������������ ����� ��� ������ � �������.
#include "config.h"

// 14 ��� - ��� �����, 13 ��� �� ������ �� ������ ��� ������ ���������� = 8191.
#define __AMPLITUDE_MAX__ 8191 // ���������, ������������ �������� ���������, ���� �������� ����������� ����������� �������
// ����� ����� (������ ��������� ������ ��� ��� ������������, fifo ������ ��� 4��. �.�. �� ������ 1024 �������� ��� ������� �� ���� ������������ �������).
#define __COUNT_MAX__ 1024 // ���������, ������������ ������������� ����� ������������� ���������� ��������� ������������ ��������

namespace parus {

	#pragma pack(push, 1)
	struct adcChannel {
		unsigned short b0: 1;  // ��������������� � 1 � ������ ������� ����� ��� ������� ������ ��� (�.�. ����� ������ ��� ��� ����� ������� ������ � �����-������� ������).
		unsigned short b1: 1;  // ��������������� � 1 � �������, ������� ������������� ���������� ������������� ����� (�� ������� ������������ ������� ������ � ������ ��������������) ��� ������� ������ ���.    
		short value : 14; // ������ �� ������ ������ ���, 14-��������� �����
	};
	
	struct adcTwoChannels {
		adcChannel re; // ������� ������ ����������
		adcChannel im; // ������� ������ ����������
	};

	struct ionPackedData { // ����������� ������ ����������.
		unsigned size; // ������ ����������� ���������� � ������.
		unsigned char *ptr;   // ��������� �� ���� ������ ����������� ����������.
	};

	/* =====================================================================  */
	/* ������ ��������� ������ ���-������� */
	/* =====================================================================  */
	/* ������ ������ ���������� � ��������� ��������� ��������� */
	struct FrequencyData {
		unsigned short frequency; //!< ������� ������������, [���].
		unsigned short gain_control; // !< �������� ���������� �������� ����������� ��.
		unsigned short pulse_time; //!< ����� ������������ �� ����� �������, [��].
		unsigned char pulse_length; //!< ������������ ������������ ��������, [��c].
		unsigned char band; //!< ������ �������, [���].
		unsigned char type; //!< ��� ��������� (0 - ������� �������, 1 - ���).
		unsigned char threshold_o; //!< ����� ��������� ������������ �����, ���� �������� ������� �� ����� ������������ � ����, [��/��. ���].
		unsigned char threshold_x; //!< ����� ��������� �������������� �����, ���� �������� ������� �� ����� ������������ � ����, [��/��. ���].
		unsigned char count_o; //!< ����� �������� ���������� O.
		unsigned char count_x; //!< ����� �������� ���������� X.
	};

	/* ������� ������� FrequencyData::count_o �������� SignalResponse, ����������� ������������ �����. */
	/* C���� �� ����� ������������ ���� SignalResponse  � SignalSample  ��� ������������ �������� ������� FrequencyData::count_x */
	/* �������� SignalResponse, ����������� �������������� ������� � �������� �������� SignalSample ����� ������ �� ���. �������� */
	/* FrequencyData::count_o � FrequencyData::count_x ����� ���� ����� ����, ����� ��������������� ������ �����������. */
	struct SignalResponse {
		unsigned long height_begin; //!< ��������� ������, [�]
		unsigned short count_samples; //!< ����� ���������
	};

	/* =====================================================================  */
	#pragma pack(pop)

	// ����� ������ � ����������� ������.
	class parusWork {

		HANDLE _hFile; // �������� ���� ������

		unsigned int _g;
		char _att;
		unsigned int _fsync;
		unsigned int _pulse_duration;
		unsigned int _curFrq;
    
		unsigned _height_step; // ��� �� ������, �
		unsigned _height_count; // ���������� �����
		unsigned _height_min; // ��������� ������, �� (��, ��� ���� ��� ��������� �������������)

		int _RetStatus;
		M214x3M_DRVPARS _DrvPars;
		int _DAQ; // ���������� ���������� (���� ����� ����, �� ������ ��������� ������)
		DAQ_ASYNCREQDATA *_ReqData; // ������� ������� ���
		DAQ_ASYNCXFERDATA _xferData; // ��������� ������������ ������
		DAQ_ASYNCXFERSTATE _curState;

		unsigned long *_fullBuf; // ������� ������ ������
		unsigned long buf_size;

		BYTE *getBuffer(void){return (BYTE*)(_ReqData->Tbl[0].Addr);}
		DWORD getBufferSize(void){return (DWORD)(_ReqData->Tbl[0].Size);}

		HANDLE _LPTPort; // ��� ���������������� �����������
		HANDLE initCOM2(void);
		void initLPT1(void);

		void closeOutputFile(void);

		// ������ � ������������
		unsigned short *_sum_abs; // ������ ���������� ��������

		// ������ � ������ �������
		std::vector<std::string> _log;

	public:
		// ���������� ��������
		static const std::string _PLDFileName;
		static const std::string _DriverName;
		static const std::string _DeviceName;
		static const double _C; // �������� ����� � �������

		parusWork(void);
		~parusWork(void);

		M214x3M_DRVPARS initADC(unsigned int nHeights);

		void ASYNC_TRANSFER(void);
		int READ_BUFISCOMPLETE(unsigned long msTimeout);
		void READ_ABORTIO(void);
		void READ_GETIOSTATE(void);
		int READ_ISCOMPLETE(unsigned long msTimeout);

		void setup(xml_unit* conf);
		void adjustSounding(unsigned int curFrq);
		void startGenerator(unsigned int nPulses);

		// ������ � ������������
		void openIonogramFile(xml_ionogram* conf);
		void cleanLineAccumulator(void);
		void accumulateLine(unsigned short curFrq); // ������������ �� ��������� �� ����� �������
		void averageLine(unsigned pulse_count); // ���������� �� ��������� �� ����� �������
		unsigned char getThereshold(unsigned char *arr, unsigned n);
		void saveLine(unsigned short curFrq); // �������� ������ �� char (����� �� 6 ���) � ���������� ����� � �����.
		void saveDirtyLine(void);

		// ������ � ������� �������� ������
		void openDataFile(xml_amplitudes* conf);
		void saveFullData(void);
		void saveDataWithGain(void);

		// ������ � ������ �������
		std::vector<std::string> getLog(void){ return _log;}
	};

	int comp(const void *i, const void *j);

} // namespace parus

#endif // __PARUS_H__