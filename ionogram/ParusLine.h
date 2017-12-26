// ===========================================================================
// ������������ ���� ��� ������ � ������ ���.
// ===========================================================================
#ifndef __LINEADC_H__
#define __LINEADC_H__

#include <windows.h>

#include <algorithm>
#include <numeric>
#include <string>
#include <stdexcept>
#include <climits>
#include <cmath>
#include <vector>

#include "ParusConfig.h"
#include "ParusException.h"

// 14 ��� - ��� �����, 13 ��� �� ������ �� ������ ��� ������ ���������� = 8191.
#define __AMPLITUDE_MAX__ 8190 // ���������, ������������ �������� ���������, ���� �������� ����������� ����������� �������
// ����� ����� (������ ��������� ������ ��� ��� ������������, fifo ������ ��� 4��. �.�. �� ������ 1024 �������� ��� ������� �� ���� ������������ �������).
#define __COUNT_MAX__ 1024 // ���������, ������������ ������������� ����� ������������� ���������� ��������� ������������ ��������

#endif // __LINEADC_H__

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

	// ���������, �������� �������������� �������.
	struct Statistics
	{
		double Q1, Q3, dQ; // �������� � �������������� ��������
		double thereshold; // ������� ������� ��������
		double median; // �������
		double median_top_half; // ������� ������� �������� ������� - ��� ����������� �������� 0
		double mean; // ������� ��������
		double std_mean; // ����������� ���������� (������ �� ���������) �� ��������
		double std_median; // ����������� ���������� (������ �� ���������) �� �������
		double min, max;
	};

	struct CLineBuf
	{
		unsigned count; // ����� ���� � ������� ��� ������ ������
		unsigned char *arr; // ��������� �� ������ �������������� ��� ������ � ���� ������
		int shift_0; // �������� (������ ����� - �������) �� 0 ������� ������ (Re)
		int shift_1; // �������� (������ ����� - �������) �� 0 ������� ������ (Im)

		CLineBuf(void) : count(__COUNT_MAX__) {arr = new unsigned char [count];}
		~CLineBuf(void){delete [] arr;}
	};

	// ��������� ������, ���������� ���.
	class lineADC
	{
		std::vector<int> _re, _im;
		std::vector<double> _abs;
		unsigned long *_buf; // ��������� �� ����� ����������� ������
		unsigned char *_buf_ionogram; // ��������� �� ����� ������ ����������
		unsigned long *_buf_amplitudes; // ��������� �� ����� ������ ��������

		size_t _real_buf_size; // ������ ������ ��� ���������� ����������� (��������� ������ ��������� �����)
		double zero_shift_re;
		double zero_shift_im;

		template<typename T>
		double calculateZeroShift(const std::vector<T>& vec);

		double calculateAbsThereshold(unsigned char* vec);

	public:
		lineADC(unsigned _height_count);
		lineADC(BYTE *buf);
		~lineADC();

		void accumulate(BYTE *buf);
		void average(unsigned pulse_count);
		void setSavedSize(size_t size){_real_buf_size = size;}
		size_t getSavedSize(void){return _real_buf_size;}
		unsigned long* getBufer(void){return _buf;}
		
		void prepareIPG_IonogramBuffer(xml_ionogram* ionogram, const unsigned short curFrq);
		void prepareDirty_IonogramBuffer();
		void prepareDirty_AmplitudesBuffer();

		//template<typename T>
		//Statistics calculateStatistics(const std::vector<T>& vec);

		// get
		short getShiftRe(){return static_cast<short>(zero_shift_re);}
		short getShiftIm(){return static_cast<short>(zero_shift_im);}
		char* returnIonogramBuffer(){return reinterpret_cast<char*>(_buf_ionogram);}
		char* returnAmplitudesBuffer(){return reinterpret_cast<char*>(_buf_amplitudes);}
	};

} // namespace parus