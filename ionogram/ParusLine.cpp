#include "ParusLine.h"

namespace parus {

	// Class lineADC
	// ������������� ��� ����������.
	lineADC::lineADC(unsigned _height_count = __COUNT_MAX__/2) :
		_buf(nullptr),
		_buf_ionogram(nullptr),
		_real_buf_size(_height_count),
		zero_shift_re(0),
		zero_shift_im(0)
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
		accumulate(buf);
	}

	// ������ ������ ������
	lineADC::~lineADC()
	{
		if(_buf) delete [] _buf;
		if(_buf_ionogram) delete [] _buf_ionogram;
	}

	// ���������� �������������� �������� �� ����������� ������.
	void lineADC::accumulate(BYTE *buf)
	{
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
			_re.at(i) = twoCh.re.value>>2;
	        _im.at(i) = twoCh.im.value>>2;
			
			// ��������� ��������� � ������������ � ���������.
			_abs.at(i) += sqrt(pow(static_cast<double>(_re.at(i)),2) + pow(static_cast<double>(_im.at(i)),2));
		
			// �������� ���������� ��� ��������� ��������� ��������, ����� ��������� ����������� ��������.
			// � ������� ��������� ������������� ���������� �������, �� ������� ��������� ����������.
			bool isRe = abs(_re.at(i)) >= __AMPLITUDE_MAX__;
			bool isIm = abs(_im.at(i)) >= __AMPLITUDE_MAX__;
			if(isRe || isIm)
				throw CADCOverflowException(i, isRe, isIm);
		}
		zero_shift_re += calculateZeroShift(_re);
		zero_shift_im += calculateZeroShift(_im);
	}

	// ���������� ��������� �������.
	void lineADC::average(unsigned pulse_count)
	{
		for(size_t i = 0; i < _abs.size(); i++)
			_abs.at(i) /= pulse_count;
		// �������� �������� ����
		zero_shift_re /= pulse_count;
		zero_shift_im /= pulse_count;
	}

	// �������� ������ �� char (����� �� 6 ���) � ���������� ����� � �����.
	// �������� ������ ���������� �� ��������� �������-���.
	void lineADC::prepareIPG_IonogramBuffer(xml_ionogram* ionogram, const unsigned short curFrq)
	{
		// ������������ ������ �� ������� �������
		FrequencyData tmpFrequencyData;
		tmpFrequencyData.gain_control = static_cast<unsigned short>(ionogram->getGain());// !< �������� ���������� �������� ����������� ��.
		tmpFrequencyData.pulse_time = static_cast<unsigned short>(floor(1000./ionogram->getPulseFrq()));//!< ����� ������������ �� ����� �������, [��].
		tmpFrequencyData.pulse_length = static_cast<unsigned char>(ionogram->getPulseDuration());//!< ������������ ������������ ��������, [��c].
		tmpFrequencyData.band = static_cast<unsigned char>(floor(1000000./ionogram->getPulseDuration()));//!< ������ �������, [���].
		tmpFrequencyData.type = 0;														//!< ��� ��������� (0 - ������� �������, 1 - ���).

		// ������� ����� ��� ����������� ������. �������, ��� ����������� ������ �� ������ ��������.
		unsigned n = __COUNT_MAX__;
		unsigned char *tmpArray = new unsigned char [n];
		unsigned char *tmpLine = new unsigned char [n];
		unsigned char *tmpAmplitude = new unsigned char [n];
  
		unsigned j;
		unsigned char *dataLine = new unsigned char [n];

		// �������� ������ �� ������� 8 ���.
		for(unsigned i = 0; i < n; i++) 
			dataLine[i] = static_cast<unsigned char>(static_cast<unsigned short>(_abs.at(i)) >> 6);

		SignalResponse tmpSignalResponse;
		unsigned char countOutliers;
		unsigned short countFrq = 0; // ���������� ���� � ����������� ��������� �������

		// ��������� ������� ������� ��������.
		unsigned char thereshold = static_cast<unsigned char>(calculateAbsThereshold(dataLine));           
		tmpFrequencyData.frequency = curFrq;
		tmpFrequencyData.threshold_o = thereshold;
		tmpFrequencyData.threshold_x = 0;
		tmpFrequencyData.count_o = 0; // ����� ����������
		tmpFrequencyData.count_x = 0; // ������� � ��� ���� �- � �- ���������� ����������.

		unsigned short countLine = 0; // ���������� ���� � ����������� ������
		countOutliers = 0; // ������� ���������� �������� � ������� ������
		j = 0;
		while(j < n) // ������� �������
		{
			if(dataLine[j] > thereshold) // ������ ������ - ������������ ���.
			{
				tmpSignalResponse.height_begin = static_cast<unsigned long>(floor(1.0 * j * ionogram->getHeightStep()));
				tmpSignalResponse.count_samples = 1;
				tmpAmplitude[tmpSignalResponse.count_samples-1] = dataLine[j];
				j++; // ������� � ���������� ��������
				while(dataLine[j] > thereshold && j < n)
				{
					tmpSignalResponse.count_samples++;
					tmpAmplitude[tmpSignalResponse.count_samples-1] = dataLine[j];
					j++; // ������� � ���������� ��������
				}
				countOutliers++; // ��������� ���������� �������� � ������

				// �������� ����������� �������
				// ��������� �������.
				unsigned short nn = sizeof(SignalResponse);
				memcpy(tmpLine+countLine, &tmpSignalResponse, nn);
				countLine += nn;
				// ������ �������.
				nn = tmpSignalResponse.count_samples*sizeof(unsigned char);
				memcpy(tmpLine+countLine, tmpAmplitude, nn);
				countLine += nn;
			}
			else
				j++; // ���� ��������� ������
		}

		// ������ ��� ������� ������� �������� � �����.
		// ��������� ������ ������� ������, ���� ���� ������� �����������.
		tmpFrequencyData.count_o = countOutliers;
		// ��������� �������.
		unsigned nFrequencyData = sizeof(FrequencyData);
		memcpy(tmpArray, &tmpFrequencyData, nFrequencyData);
		// ��������� ����� �������������� ������� ��������
		if(countLine)
			memcpy(tmpArray+nFrequencyData, tmpLine, countLine);

		// ��������� �������� ��������� � ������ ������.
		setSavedSize(countLine + nFrequencyData);
		_buf_ionogram = new unsigned char [getSavedSize()];
		memcpy(_buf_ionogram, reinterpret_cast<char*>(tmpArray), getSavedSize());

		delete [] tmpLine;
		delete [] tmpAmplitude;
		delete [] tmpArray;
		delete [] dataLine;
	} 
	
	void lineADC::prepareDirty_IonogramBuffer()
	{
		// �������� ������ �� ������� 8 ���.
		for(size_t i = 0; i < getSavedSize(); i++) 
			_buf_ionogram[i] = static_cast<unsigned char>(static_cast<unsigned short>(_abs.at(i)) >> 6);
	}

	/// <summary>
	/// Prepares the dirty amplitudes buffer. ������� ����������� ������. ������������ ������ ������ �������� ��������.
	/// </summary>
	void lineADC::prepareDirty_AmplitudesBuffer()
	{
		memcpy(_buf_amplitudes, _buf, sizeof(unsigned long) * getSavedSize());
	}

	/// <summary>
	/// ��������� �������� ���� �� ������� �������� ������� ���������.
	/// </summary>
	/// <param name="vec">������� ������.</param>
	/// <returns>�������� �������� ����.</returns>
	template<typename T>
	double lineADC::calculateZeroShift(const std::vector<T>& vec)
	{
		double _out;
		int n = vec.size();
		std::vector<T> vec_half;
		vec_half.insert(vec_half.begin(),vec.begin()+(n/2-1),vec.end()); // ������� �������� �������

		// ��������� �� ����������� � ���������� �������.
		sort(vec_half.begin(), vec_half.end());
		_out = (vec_half.at(n/2),vec_half.at(n/2-1))/2.;

		return _out;
	}

	/// <summary>
	/// ��������� ��������� �������, ���� ��������, ����������������, �������������� ������ ���������.
	/// </summary>
	/// <param name="vec">������� ������ ���������� �������� ��������.</param>
	/// <returns>�������� ������.</returns>
	double lineADC::calculateAbsThereshold(unsigned char* _in)
	{
		double thereshold;
		unsigned n = __COUNT_MAX__;
		std::vector<double> vec;

		// 0. �������� ������� ������.
		for(size_t i = 0; i < n; i++) 
			vec.at(i) = _in[i];
		// 1. ���������� ������ �� �����������.
		sort(vec.begin(), vec.end());
		// 2. �������� (n - ������, ������� ������!!!)
		double Q1 = (vec.at(n/4)+vec.at(n/4-1))/2.;
		double Q3 = (vec.at(3*n/4),vec.at(3*n/4-1))/2.;
		// 3. �������������� ��������
		double dQ = Q3 - Q1;
		// 4. ������� ������� ��������.
		thereshold = Q3 + 3 * dQ;

		return thereshold;
	}

} // namespace parus

	//// ���������� ���������� ������ ������ (short/double).
	//template<typename T>
	//Statistics calculateStatistics(const std::vector<T>& vec)
	//{
	//	Statistics _out;
	//	int n = vec.size();

	//	std::vector<T> vec_half;
	//	vec_half.insert(vec_half.begin(),vec.begin()+(n/2-1),vec.end()); // ������� �������� �������

	//	// 1. ���������� ������ �� �����������.
	//	sort(vec.begin(), vec.end());
	//	sort(vec_half.begin(), vec_half.end());
	//	// 2. �������� (n - ������, ������� ������!!!)
	//	_out.Q1 = (vec.at(n/4)+vec.at(n/4-1))/2.;
	//	_out.median = (vec.at(n/2),vec.at(n/2-1))/2.;
	//	_out.median_top_half = (vec_half.at(n/2),vec_half.at(n/2-1))/2.;
	//	_out.Q3 = (vec.at(3*n/4),vec.at(3*n/4-1))/2.;
	//	// 3. �������������� ��������
	//	_out.dQ = _out.Q3 - _out.Q1;
	//	// 4. ������� ������� ��������.
	//	_out.thereshold = _out.Q3 + 3 * _out.dQ;

	//	// ������� ��������
	//	_out.mean = std::accumulate(vec.begin(), vec.end(), 0) / n;

	//	// ����������� ���������� �� �������� ��������
	//	std::vector<double> diff(n);
	//	std::transform(vec.begin(), vec.end(), diff.begin(),
 //              std::bind2nd(std::minus<double>(), _out.mean));
	//	double sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
	//	_out.std_mean = std::sqrt(sq_sum /(n-1));

	//	// ����������� ���������� �� �������
	//	std::vector<double> diff(n);
	//	std::transform(vec.begin(), vec.end(), diff.begin(),
 //              std::bind2nd(std::minus<double>(), _out.median));
	//	double sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
	//	_out.std_median = std::sqrt(sq_sum /(n-1));

	//	// ����������� � ������������ ��������
	//	_out.min = vec.front();
	//	_out.max = vec.back();

	//	return _out;
	//}