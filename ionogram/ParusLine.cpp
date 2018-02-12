#include "ParusLine.h"

namespace parus {

	// *************************************************************************
	// Class CBuffer
	// ��������� ������ ��� � ���������� ��� ������ � ���� �������� WriteFile.
	// *************************************************************************
	
	/// <summary>
	/// Initializes a new instance of the <see cref="CBuffer"/> class.
	/// �� ���������:
	///	saved_count_ = __COUNT_MAX__/2 (����� �����, ����������� � ����),
	/// ��� ������� ����������� ������. ������ �������� = __COUNT_MAX__.
	/// </summary>
	CBuffer::CBuffer() : 
		saved_count_(__COUNT_MAX__/2),
		re_(__COUNT_MAX__,0),
		im_(__COUNT_MAX__,0),
		abs_(__COUNT_MAX__,0),
		ArrayToFile_(nullptr),
		BytesCountToFile_(0)
	{} 

	/// <summary>
	/// Initializes a new instance of the <see cref="CBuffer"/> class.
	/// </summary>
	/// <param name="obj">������ ������ CBuffer.</param>
	CBuffer::CBuffer(const CBuffer& obj) :
		saved_count_(obj.getSavedSize()),
		ArrayToFile_(nullptr),
		BytesCountToFile_(0)
	{
		re_.assign(obj.re_.begin(),obj.re_.end());
		im_.assign(obj.im_.begin(),obj.im_.end());
		abs_.assign(obj.abs_.begin(),obj.abs_.end());
	}

	/// <summary>
	/// Initializes a new instance of the <see cref="CBuffer"/> class.
	/// </summary>
	/// <param name="adc">��������� �� �������� ����� ���.</param>
	/// <param name="count">����� ����� (32-���) � �������� ������.</param>
	/// <param name="saved_count">����� ����� ��� ����������.</param>
	/// �� ���������:
	/// count_ = __COUNT_MAX__ (����� ����� �� ������� ������),
	///	saved_count_ = __COUNT_MAX__/2 (����� �����, ����������� � ����).
	CBuffer::CBuffer(BYTE* adc, unsigned count,	unsigned saved_count) : 
		saved_count_(saved_count),
		re_(count,0),
		im_(count,0),
		abs_(count,0),
		ArrayToFile_(nullptr),
		BytesCountToFile_(0)
	{
		accumulate(adc);
	}

	/// <summary>
	/// Finalizes an instance of the <see cref="CBuffer"/> class.
	/// </summary>
	CBuffer::~CBuffer()
	{
		if(ArrayToFile_ != nullptr)
		{
			delete [] ArrayToFile_;
			ArrayToFile_ = nullptr;
		}
	}

	/// <summary>
	/// ������������ ��������� ����� ��� � �������. ���������� ����� ���
	/// ����������� � count_.
	/// </summary>
	/// <param name="adc">��������� �� ����� ���.</param>
	void CBuffer::accumulate(BYTE* adc)
	{
		unsigned long *_tmp;
		_tmp = new unsigned long [re_.size()];
		// � ������� ��� ��������� ��� ����� 16-������ ������������ ������ �� 
		// ����� ������� (count - ���������� 32-��������� ����).
		memcpy(_tmp, adc, re_.size()*4); // �������� ���� ���������� �����

		for(size_t i = 0; i < re_.size(); i++)
		{
	        // ���������� ������������� ������������� ����� ��������� ���������
	        union {
	            unsigned long word;     // 4-������� ����� �������������� ���
	            adcTwoChannels twoCh;  // ������������� (������������) ������
	        };
	            
			// ��������� �� ����������. ������� ������ ������� 14 ���.
	        word = _tmp[i];
			re_.at(i) = twoCh.re.value; // ��� ������� �� 2 ����� ������
	        im_.at(i) = twoCh.im.value; // ��� ������� �� 2 ����� ������
			abs_.at(i) += sqrt(
				pow(static_cast<double>(re_.at(i)),2) + 
				pow(static_cast<double>(im_.at(i)),2)
				);
		
			// �������� ���������� ��� ��������� ��������� ��������, ����� 
			// ��������� ����������� ��������. � ������� ��������� ������������� 
			// ���������� LOGa �������, �� ������� ��������� ����������.
			// !!! ������ ����� ����������� �� ��������� !!!
			bool isRe = abs(re_.at(i)) >= __AMPLITUDE_MAX__;
			bool isIm = abs(im_.at(i)) >= __AMPLITUDE_MAX__;
			if(isRe || isIm)
				throw CADCOverflowException(i, isRe, isIm);
		}
		delete [] _tmp;
	}

	/// <summary>
	/// Operator=s the specified object.
	/// </summary>
	/// <param name="obj">The object CBuffer class.</param>
	/// <returns></returns>
	CBuffer& CBuffer::operator=(CBuffer& obj)
	{
		if (this == &obj) // �������� �� �������������������
			return *this;

		saved_count_ = obj.getSavedSize();
		re_.assign(obj.re_.begin(),obj.re_.end());
		im_.assign(obj.im_.begin(),obj.im_.end());
		abs_.assign(obj.abs_.begin(),obj.abs_.end());

		return *this; // ���������� ������ �� ������� ������
	}
	
	/// <summary>
	/// Operator+= ������������ ���������� ��������� �������.
	/// ����������� ��������� ������������ ���������.
	/// </summary>
	/// <param name="adc">����� ������ �������� ���.</param>
	/// <returns></returns>
	CBuffer& CBuffer::operator+=(BYTE* adc)
	{
		accumulate(adc);
		return *this; // ���������� ������ �� ������� ������
	}
	
	/// <summary>
	/// Operator/= ���������� ������� ������������ ������� ���������� ��������
	/// �� ����� � ��������� -127..+127.
	/// </summary>
	/// <param name="value">�������� ���� char.</param>
	/// <returns></returns>
	CBuffer& CBuffer::operator/=(char value)
	{
		if(value)
			for (auto it = abs_.begin(); it != abs_.end(); ++it)
				*it /= value;
		return *this; // ���������� ������ �� ������� ������
	}

	/// <summary>
	/// ���������� �������� ���� ��� ������� ������������� ����.
	/// ���������� ������ ������� �������� �������, ��� ��������� ����.
	/// </summary>
	/// <param name="vec">������ ������������� ����.</param>
	/// <returns>��� double.</returns>
	template<typename T>
	double const CBuffer::calculate_zero_shift(const std::vector<T>& vec) const
	{
		double tmp;

		int n = vec.size()/2;
		std::vector<short> top_half;
		top_half.insert(top_half.begin(),vec.begin()+n,vec.end()); 

		// ��������� �� ����������� � ���������� �������.
		sort(top_half.begin(), top_half.end());
		tmp = (top_half.at(n/2) + top_half.at(n/2-1))/2.;

		return tmp;
	}

	template<typename T>
	double const CBuffer::calculate_thereshold(const T* _in, size_t count) const
	{
		double thereshold;
		std::vector<double> vec(count,0);

		// 0. �������� ������� ������.
		for(size_t i = 0; i < count; i++) 
			vec.at(i) = _in[i];
		// 1. ���������� ������ �� �����������.
		sort(vec.begin(), vec.end());
		// 2. �������� (n - ������, ������� ������!!!)
		double Q1 = (vec.at(count/4)+vec.at(count/4-1))/2.;
		double Q3 = (vec.at(3*count/4),vec.at(3*count/4-1))/2.;
		// 3. �������������� ��������
		double dQ = Q3 - Q1;
		// 4. ������� ������� ��������.
		thereshold = Q3 + 3 * dQ;

		return thereshold;
	}

	/// <summary>
	/// ���������� �������� ���� ��� ����� �������.
	/// </summary>
	/// <returns>��� CPoint.</returns>
	CPoint CBuffer::getZeroShift() const
	{
		CPoint out;
		out.re = static_cast<short>(calculate_zero_shift(re_));
		out.im = static_cast<short>(calculate_zero_shift(im_));

		return out;
	}

	/// <summary>
	/// ���������� �������� ���������� ��� �������� ������ ��� ������ � ����.
	/// ���������� private �������:
	/// BYTE* ArrayToFile_;
	/// size_t BytesCountToFile_;
	/// </summary>
	void CBuffer::prepareIonogram_Dirty()
	{
		// �������� �� ������������� � ���������, ���� �������������.
		if(ArrayToFile_ != nullptr)
		{
			delete [] ArrayToFile_;
			ArrayToFile_ = nullptr;
		}

		unsigned short *dataLine = new unsigned short [getSavedSize()];

		// �������� ������ �� ������� 8 ���.
		for(size_t i = 0; i < getSavedSize(); i++) 
			dataLine[i] = static_cast<unsigned short>(abs_.at(i));
		BytesCountToFile_ = 2 * getSavedSize(); // ���������� short �����

		ArrayToFile_ = new BYTE [BytesCountToFile_];
		memcpy(ArrayToFile_, reinterpret_cast<BYTE*>(dataLine), BytesCountToFile_);

		delete [] dataLine;
	}

	/// <summary>
	/// ���������� ���������� � ������� ��� ��� ������ � ����.
	/// ���������� private �������:
	/// BYTE* ArrayToFile_;
	/// size_t BytesCountToFile_;
	/// </summary>
	/// <param name="ionogram">const xml_ionogram& - ��������� ������������ ��� ����������.</param>
	/// <param name="curFrq">const unsigned short - ������� ������� ������������, ���.</param>
	void CBuffer::prepareIonogram_IPG(const xml_ionogram& ionogram, const unsigned short curFrq)
	{
//		try{
		// ������������ ������ �� ������� �������
		FrequencyData tmpFrequencyData;
		tmpFrequencyData.gain_control = static_cast<unsigned short>(ionogram.getGain());// !< �������� ���������� �������� ����������� ��.
		tmpFrequencyData.pulse_time = static_cast<unsigned short>(floor(1000./ionogram.getPulseFrq()));//!< ����� ������������ �� ����� �������, [��].
		tmpFrequencyData.pulse_length = static_cast<unsigned char>(ionogram.getPulseDuration());//!< ������������ ������������ ��������, [��c].
		tmpFrequencyData.band = static_cast<unsigned char>(floor(1000000./ionogram.getPulseDuration()));//!< ������ �������, [���].
		tmpFrequencyData.type = 0;														//!< ��� ��������� (0 - ������� �������, 1 - ���).

		unsigned j;
		unsigned n = getSavedSize();
		unsigned char *tmpArray;
		unsigned char *tmpLine;
		unsigned char *tmpAmplitude;
		unsigned char *dataLine;

		// ������� ����� ��� ����������� ������. �������, ��� ����������� ������ �� ������ ��������.
		tmpArray = new unsigned char [n];
		tmpLine = new unsigned char [n];
		tmpAmplitude = new unsigned char [n];
		dataLine = new unsigned char [n];

		// �������� ������ �� ������� 8 ��� (�� 2 ��� ��� �������� ������).
		for(unsigned i = 0; i < n; i++) 
			dataLine[i] = static_cast<unsigned char>(static_cast<unsigned short>(abs_.at(i)) >> 6);

		SignalResponse tmpSignalResponse;
		unsigned char countOutliers = 0; // ������� ���������� �������� � ������� ������
		unsigned short countLine = 0; // ���������� ���� � ����������� ������
		unsigned short countFrq = 0; // ���������� ���� � ����������� ��������� �������

		// ��������� ������� ������� ��������.
		unsigned char thereshold = static_cast<unsigned char>
			(floor(0.5 + calculate_thereshold(dataLine, n))); // ����������� round!!!
		tmpFrequencyData.frequency = curFrq;
		tmpFrequencyData.threshold_o = thereshold;
		tmpFrequencyData.threshold_x = 0;
		tmpFrequencyData.count_o = 0; // ����� ����������
		tmpFrequencyData.count_x = 0; // ������� � ��� ���� �- � �- ���������� ����������.
		
		j = 0;
		while(j < n) // ������� �������
		{
			if(dataLine[j] > thereshold) // ������ ������ - ������������ ���.
			{
				tmpSignalResponse.height_begin = static_cast<unsigned long>(floor(1.0 * j * ionogram.getHeightStep()));
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

		// �������� �� ������������� � ���������, ���� �������������.
		if(ArrayToFile_ != nullptr)
		{
			delete [] ArrayToFile_;
			ArrayToFile_ = nullptr;
		}
		// �������� ��������� �������.
		BytesCountToFile_ = countLine + nFrequencyData;
		ArrayToFile_ = new BYTE [BytesCountToFile_];
		memcpy(ArrayToFile_, reinterpret_cast<BYTE*>(tmpArray), BytesCountToFile_);

		delete [] tmpLine;
		delete [] tmpAmplitude;
		delete [] tmpArray;
		delete [] dataLine;

				//				}
				//catch(const std::bad_alloc& e) // ����������� ����� ������ ������ ����������� ���������.
				//{
				//	std::cout << e.what();
				//}
	}

	void CBuffer::prepareAmplitudes_Dirty()
	{

	}

	void CBuffer::prepareAmplitudes_IPG()
	{

	}

	// *************************************************************************
	// ����� Class CBuffer
	// *************************************************************************

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