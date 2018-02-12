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

	struct adcChannel 
	{
		unsigned short b0: 1;  // ��������������� � 1 � ������ ������� ����� ��� ������� ������ ��� (�.�. ����� ������ ��� ��� ����� ������� ������ � �����-������� ������).
		unsigned short b1: 1;  // ��������������� � 1 � �������, ������� ������������� ���������� ������������� ����� (�� ������� ������������ ������� ������ � ������ ��������������) ��� ������� ������ ���.    
		short value : 14; // ������ �� ������ ������ ���, 14-��������� �����
	};
	
	struct adcTwoChannels 
	{
		adcChannel re; // ������� ������ ����������
		adcChannel im; // ������� ������ ����������
	};

	struct ionPackedData // ����������� ������ ����������.
	{ 
		unsigned size; // ������ ����������� ���������� � ������.
		unsigned char *ptr;   // ��������� �� ���� ������ ����������� ����������.
	};

	/* =====================================================================  */
	/* ������ ��������� ������ ���-������� */
	/* =====================================================================  */
	/* ������ ������ ���������� � ��������� ��������� ��������� */
	struct FrequencyData 
	{
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
	struct SignalResponse 
	{
		unsigned long height_begin; //!< ��������� ������, [�]
		unsigned short count_samples; //!< ����� ���������
	};

	/* =====================================================================  */
	#pragma pack(pop)

	// ���������, �������� �������������� �������.
	struct CStatistics
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
	
	struct CPoint 
	{
		short re; // ������� ������ ����������
		short im; // ������� ������ ����������
	};

	// =========================================================================
	// ����� ����� ��� ������ � ������� ���. 28.12.2017.
	class CBuffer
	{
		// ���������� ����� ������ (������� � �������� ������ ������), 
		// ���������������� ��� ���������� � ����
		unsigned saved_count_;

		// ������
		std::vector<short> re_, im_; // 13 ��� �� ������
		std::vector<double> abs_;

		BYTE* ArrayToFile_; // ������, �������������� ��� ����������
		size_t BytesCountToFile_; // ���������� ���� � ������� ��� ����������
	
		void accumulate(BYTE* adc);
		template<typename T>
		double const calculate_zero_shift(const std::vector<T>& vec) const;
		template<typename T>
		double const calculate_thereshold(const T* arr, size_t count) const;
	public:
		CBuffer();
		CBuffer(const CBuffer& obj);
		CBuffer(
			BYTE* adc,
			unsigned count = __COUNT_MAX__, 
			unsigned saved_count = __COUNT_MAX__/2);
		virtual ~CBuffer();

		// get
		unsigned getFullSize() const {return re_.size();}
		unsigned getSavedSize() const {return saved_count_;}
		CPoint getZeroShift() const;

		// set
		void setSavedSize(const unsigned saved_count){saved_count_ = saved_count;}

		// operation
		CBuffer& operator=(CBuffer& obj);
		CBuffer& operator+=(BYTE* adc);
		CBuffer& operator/=(char value);

		// ���������� ������� � ������ � ����
		size_t getBytesCountToFile() const {return BytesCountToFile_;}
		BYTE* getBytesArrayToFile() const {return ArrayToFile_;}

		void prepareIonogram_Dirty();
		void prepareIonogram_IPG(const xml_ionogram& ionogram, const unsigned short curFrq);
		void prepareAmplitudes_Dirty(unsigned short curFrq);
		void prepareAmplitudes_IPG();
	};
	// =========================================================================

} // namespace parus