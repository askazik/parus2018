#include "ParusLine.h"

namespace parus {

	// Class lineADC
	// ������������� ��� ����������.
	lineADC::lineADC() :
		_buf(nullptr),
		_saved_buf_size(__COUNT_MAX__)
	{
		// ������� ������ �������. ������ ������ ���� ���!!!
		_re.resize(__COUNT_MAX__);
		_im.resize(__COUNT_MAX__);
		_abs.resize(__COUNT_MAX__);
		
		_buf = new unsigned long [__COUNT_MAX__];
		memset(_buf, 0, static_cast<unsigned long>(__COUNT_MAX__ * sizeof(unsigned long))); // ������ ������ ������ � ������;
	}

	// ������������� � �����������.
	lineADC::lineADC(BYTE *buf)
	{
		lineADC();
		fill(buf);
	}

	// ������ ������ ������
	lineADC::~lineADC()
	{
		if(_buf) delete [] _buf;
	}

	// ���������� �������������� �������� �� ����������� ������.
	bool lineADC::fill(BYTE *buf)
	{
		// ������� ����������� �������.
		bool isLimitExceeded = false;

		// �������� �����-����� ��������� �������
		memcpy(_buf, buf, static_cast<unsigned long>(__COUNT_MAX__ * sizeof(unsigned long))); // ������ ������ ������ � ������;
		for(size_t i = 0; i < _re.size(); i++)
		{
	        // ���������� ������������� ������������� ����� ��������� ���������
	        union {
	            unsigned long word;     // 4-������� ����� �������������� ���
	            adcTwoChannels twoCh;  // ������������� (������������) ������
	        };
	            
			// ��������� �� ����������. 
			// ������� ������ ������� 14 ���. ������� 2 ��� - ��������������� �������.
	        word = buf[i];
			_re[i] = twoCh.re.value>>2;
	        _im[i] = twoCh.im.value>>2;
			// ��������� ��������� � ������������ � ���������.
			_abs[i] += static_cast<short>(sqrt(pow(static_cast<double>(_re.at(i)),2) + pow(static_cast<double>(_im.at(i)),2)));

			if(abs(_re[i]) >= __AMPLITUDE_MAX__ || abs(_im[i]) >= __AMPLITUDE_MAX__)
			{
				isLimitExceeded = true;
				// �������� ���������� ��� ��������� ��������� ��������, ����� ��������� ����������� ��������.
				// � ������� ��������� ������������� ���������� �������, �� ������� ��������� ����������.
				throw std::range_error("����������� �������.");
			}
		}

		return isLimitExceeded;
	}

	// ���������� ���������� ������ ������.
	Statistics lineADC::calculateStatistics(std::vector<short> vec)
	{
		Statistics _out;
		int n = vec.size();

		// 1. ���������� ������ �� �����������.
		sort(vec.begin(), vec.end());
		// 2. �������� (n - ������, ������� ������!!!)
		_out.Q1 = (vec.at(n/4)+vec.at(n/4-1))/2.;
		_out.median = (vec.at(n/2),vec.at(n/2-1))/2.;
		_out.Q3 = (vec.at(3*n/4),vec.at(3*n/4-1))/2.;
		// 3. �������������� ��������
		_out.dQ = _out.Q3 - _out.Q1;
		// 4. ������� ������� ��������.
		_out.maxLim = _out.Q3 + 3 * _out.dQ;

		// ������� ��������
		_out.mean = accumulate(vec.begin(), vec.end(), 0) / n;

		// ����������� ����������
		std::vector<double> diff(n);
		std::transform(vec.begin(), vec.end(), diff.begin(),
               std::bind2nd(std::minus<double>(), _out.mean));
		double sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
		_out.std = std::sqrt(sq_sum /(n-1));

		// ����������� � ������������ ��������
		_out.min = vec.front();
		_out.max = vec.back();

		return _out;
	}

	// �������� ������ �� char (����� �� 6 ���) � ���������� ����� � �����.
	// �������� ������ ���������� �� ��������� �������-���.
	// data - ����� ������ ����������
	// conf - ����������� ������������
	// numModule - ����� ������ ������������
	// curFrq - ������� ������������, ���
	IPGLine lineADC::getIPGBufer(void)
	{
		IPGLine _out;

		// ������������ ������ �� ������� �������
		FrequencyData tmpFrequencyData;
		//tmpFrequencyData.gain_control = static_cast<unsigned short>(_g * 6);			// !< �������� ���������� �������� ����������� ��.
		//tmpFrequencyData.pulse_time = static_cast<unsigned short>(floor(1000./_fsync));	//!< ����� ������������ �� ����� �������, [��].
		//tmpFrequencyData.pulse_length = static_cast<unsigned char>(_pulse_duration);	//!< ������������ ������������ ��������, [��c].
		//tmpFrequencyData.band = static_cast<unsigned char>(floor(1000000./_pulse_duration)); //!< ������ �������, [���].
		//tmpFrequencyData.type = 0;														//!< ��� ��������� (0 - ������� �������, 1 - ���).

		//// ������� ����� ��� ����������� ������. �������, ��� ����������� ������ �� ������ ��������.
		//unsigned n = _height_count;
		//unsigned char *tmpArray = new unsigned char [n];
		//unsigned char *tmpLine = new unsigned char [n];
		//unsigned char *tmpAmplitude = new unsigned char [n];
  //  
		//unsigned j;
		//unsigned char *dataLine = new unsigned char [n];

		//// �������� ������ �� ������� 8 ���.
		//for(unsigned i = 0; i < n; i++) 
		//	dataLine[i] = static_cast<unsigned char>(_sum_abs[i] >> 6);

		//unsigned char thereshold;
		//SignalResponse tmpSignalResponse;
		//unsigned char countOutliers;
		//unsigned short countFrq = 0; // ���������� ���� � ����������� ��������� �������

		//// ��������� ������� ������� ��������.
		//thereshold = getThereshold(dataLine, n);            
		//tmpFrequencyData.frequency = curFrq;
		//tmpFrequencyData.threshold_o = thereshold;
		//tmpFrequencyData.threshold_x = 0;
		//tmpFrequencyData.count_o = 0; // ����� ����������
		//tmpFrequencyData.count_x = 0; // ������� � ��� ���� �- � �- ���������� ����������.

		//unsigned short countLine = 0; // ���������� ���� � ����������� ������
		//countOutliers = 0; // ������� ���������� �������� � ������� ������
		//j = 0;
		//while(j < n) // ������� �������
		//{
		//	if(dataLine[j] > thereshold) // ������ ������ - ������������ ���.
		//	{
		//		tmpSignalResponse.height_begin = static_cast<unsigned long>(floor(1.0 * j * _height_step));
		//		tmpSignalResponse.count_samples = 1;
		//		tmpAmplitude[tmpSignalResponse.count_samples-1] = dataLine[j];
		//		j++; // ������� � ���������� ��������
		//		while(dataLine[j] > thereshold && j < n)
		//		{
		//			tmpSignalResponse.count_samples++;
		//			tmpAmplitude[tmpSignalResponse.count_samples-1] = dataLine[j];
		//			j++; // ������� � ���������� ��������
		//		}
		//		countOutliers++; // ��������� ���������� �������� � ������

		//		// �������� ����������� �������
		//		// ��������� �������.
		//		unsigned short nn = sizeof(SignalResponse);
		//		memcpy(tmpLine+countLine, &tmpSignalResponse, nn);
		//		countLine += nn;
		//		// ������ �������.
		//		nn = tmpSignalResponse.count_samples*sizeof(unsigned char);
		//		memcpy(tmpLine+countLine, tmpAmplitude, nn);
		//		countLine += nn;
		//	}
		//	else
		//		j++; // ���� ��������� ������
		//}

		//// ������ ��� ������� ������� �������� � �����.
		//// ��������� ������ ������� ������, ���� ���� ������� �����������.
		//tmpFrequencyData.count_o = countOutliers;
		//// ��������� �������.
		//unsigned nFrequencyData = sizeof(FrequencyData);
		//memcpy(tmpArray, &tmpFrequencyData, nFrequencyData);
		//// ��������� ����� �������������� ������� ��������
		//if(countLine)
		//	memcpy(tmpArray+nFrequencyData, tmpLine, countLine);

		//// Writing data from buffers into file (unsigned long = unsigned int)
		//BOOL	bErrorFlag = FALSE;
		//DWORD	dwBytesWritten = 0;	
		//bErrorFlag = WriteFile( 
		//	_hFile,									// open file handle
		//	reinterpret_cast<char*>(tmpArray),		// start of data to write
		//	countLine + nFrequencyData,				// number of bytes to write
		//	&dwBytesWritten,						// number of bytes that were written
		//	NULL);									// no overlapped structure
		//if (!bErrorFlag) 
		//	throw std::runtime_error("�� ���� ��������� ������ ���������� � ����.");
	
		//delete [] tmpLine;
		//delete [] tmpAmplitude;
		//delete [] tmpArray;
		//delete [] dataLine;

		return _out;
	} 

} // namespace parus