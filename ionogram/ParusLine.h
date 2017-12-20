// ===========================================================================
// ������������ ���� ��� ������ � ������ ���.
// ===========================================================================
#ifndef __LINEADC_H__
#define __LINEADC_H__

#include <algorithm>
#include <numeric>
#include <string>
#include <stdexcept>
#include <climits>
#include <cmath>
#include <vector>

#include <windows.h>

#include "ParusException.h"
#include "ParusWork.h"

// 14 ��� - ��� �����, 13 ��� �� ������ �� ������ ��� ������ ���������� = 8191.
#define __AMPLITUDE_MAX__ 8190 // ���������, ������������ �������� ���������, ���� �������� ����������� ����������� �������
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

	// ���������, �������� �������������� �������.
	struct Statistics
	{
		double Q1, Q3, dQ; // �������� � �������������� ��������
		double thereshold; // ������� ������� ��������
		double median; // �������
		double mean; // ������� ��������
		double std; // ����������� ���������� (������ �� ���������)
		double min, max;
	};

	struct CLineBuf
	{
		unsigned count; // ����� ���� � ������� ��� ������ ������
		unsigned char *arr; // ��������� �� ������ �������������� ��� ������ � ���� ������

		CLineBuf(void) : count(__COUNT_MAX__) {arr = new unsigned char [count];}
		~CLineBuf(void){delete [] arr;}
	};

	// ��������� ������, ���������� ���.
	class lineADC
	{
		std::vector<int> _re, _im;
		std::vector<double> _abs;
		unsigned long *_buf; // ��������� �� ����� ����������� ������
		short _saved_buf_size; // ������ ������ ��� ���������� ����������� (��������� ������ ��������� �����)
	public:
		lineADC();
		lineADC(BYTE *buf);
		~lineADC();

		void accumulate(BYTE *buf);
		void average(unsigned pulse_count);
		void setSavedSize(short size){_saved_buf_size = size;}
		short getSavedSize(void){return _saved_buf_size;}
		unsigned long * getBufer(void){return _buf;}
		CLineBuf getIPGBufer(parusWork parus, unsigned short curFrq);

		template<typename T>
		Statistics calculateStatistics(std::vector<T>& vec);
	};

} // namespace parus

#endif // __LINEADC_H__