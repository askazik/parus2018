#include "ParusLine.h"

namespace parus {

	// Class lineADC
	// Инициализация без заполнения.
	lineADC::lineADC(unsigned _height_count = __COUNT_MAX__/2) :
		_buf(nullptr),
		_buf_ionogram(nullptr),
		_real_buf_size(_height_count),
		zero_shift_re(0),
		zero_shift_im(0)
	{
		// Изменим размер вектора. Делаем только один раз!!!
		_re.resize(__COUNT_MAX__);
		_im.resize(__COUNT_MAX__);
		_abs.resize(__COUNT_MAX__);
		
		_buf = new unsigned long [__COUNT_MAX__];
		memset(_buf, 0, static_cast<unsigned long>(__COUNT_MAX__ * sizeof(unsigned long))); // размер буфера строки в байтах;
	}

	// Инициализация с заполнением.
	lineADC::lineADC(BYTE *buf)
	{
		lineADC();
		accumulate(buf);
	}

	// Чистим буферы данных
	lineADC::~lineADC()
	{
		if(_buf) delete [] _buf;
		if(_buf_ionogram) delete [] _buf_ionogram;
	}

	// Заполнение обрабатываемых векторов из аппаратного буфера.
	void lineADC::accumulate(BYTE *buf)
	{
		// Заполним буфер-копию исходными данными
		memcpy(_buf, buf, static_cast<unsigned long>(__COUNT_MAX__ * sizeof(unsigned long))); // размер буфера строки в байтах;
		for(size_t i = 0; i < _re.size(); i++)
		{
	        // Используем двухканальную интерпретацию через анонимную структуру
	        union {
	            unsigned long word;     // 4-байтное слово двухканального АЦП
	            adcTwoChannels twoCh;  // двухканальные (квадратурные) данные
	        };
	            
			// Разбиение на квадратуры. 
			// Значимы только старшие 14 бит. Младшие 2 бит - технологическая окраска.
	        word = buf[i];
			_re.at(i) = twoCh.re.value>>2;
	        _im.at(i) = twoCh.im.value>>2;
			
			// Амплитуды суммируем с находящимися в хранилище.
			_abs.at(i) += sqrt(pow(static_cast<double>(_re.at(i)),2) + pow(static_cast<double>(_im.at(i)),2));
		
			// Выбросим исключение для обработки нештатной ситуации, чтобы уменьшить коэффициент усиления.
			// В верхней процедуре предусмотреть заполнение частоты, на которой произошло исключение.
			bool isRe = abs(_re.at(i)) >= __AMPLITUDE_MAX__;
			bool isIm = abs(_im.at(i)) >= __AMPLITUDE_MAX__;
			if(isRe || isIm)
				throw CADCOverflowException(i, isRe, isIm);
		}
		zero_shift_re += calculateZeroShift(_re);
		zero_shift_im += calculateZeroShift(_im);
	}

	// Усреднение амплитуды сигнала.
	void lineADC::average(unsigned pulse_count)
	{
		for(size_t i = 0; i < _abs.size(); i++)
			_abs.at(i) /= pulse_count;
		// Усредним смещение нуля
		zero_shift_re /= pulse_count;
		zero_shift_im /= pulse_count;
	}

	// Усечение данных до char (сдвиг на 6 бит) и сохранение линии в файле.
	// Упаковка строки ионограммы по алгоритму ИЗМИРАН-ИПГ.
	void lineADC::prepareIPG_IonogramBuffer(xml_ionogram* ionogram, const unsigned short curFrq)
	{
		// Неизменяемые данные по текущей частоте
		FrequencyData tmpFrequencyData;
		tmpFrequencyData.gain_control = static_cast<unsigned short>(ionogram->getGain());// !< Значение ослабления входного аттенюатора дБ.
		tmpFrequencyData.pulse_time = static_cast<unsigned short>(floor(1000./ionogram->getPulseFrq()));//!< Время зондирования на одной частоте, [мс].
		tmpFrequencyData.pulse_length = static_cast<unsigned char>(ionogram->getPulseDuration());//!< Длительность зондирующего импульса, [мкc].
		tmpFrequencyData.band = static_cast<unsigned char>(floor(1000000./ionogram->getPulseDuration()));//!< Полоса сигнала, [кГц].
		tmpFrequencyData.type = 0;														//!< Вид модуляции (0 - гладкий импульс, 1 - ФКМ).

		// Выделим буфер под упакованную строку. Считаем, что упакованная строка не больше исходной.
		unsigned n = getSavedSize();
		unsigned char *tmpArray = new unsigned char [n];
		unsigned char *tmpLine = new unsigned char [n];
		unsigned char *tmpAmplitude = new unsigned char [n];
  
		unsigned j;
		unsigned char *dataLine = new unsigned char [n];

		// Усечение данных до размера 8 бит.
		for(unsigned i = 0; i < n; i++) 
			dataLine[i] = static_cast<unsigned char>(static_cast<unsigned short>(_abs.at(i)) >> 6);

		SignalResponse tmpSignalResponse;
		unsigned char countOutliers;
		unsigned short countFrq = 0; // количество байт в упакованном частотном массиве

		// Определим уровень наличия выбросов.
		unsigned char thereshold = static_cast<unsigned char>(calculateAbsThereshold(dataLine));           
		tmpFrequencyData.frequency = curFrq;
		tmpFrequencyData.threshold_o = thereshold;
		tmpFrequencyData.threshold_x = 0;
		tmpFrequencyData.count_o = 0; // может изменяться
		tmpFrequencyData.count_x = 0; // Антенна у нас одна о- и х- компоненты объединены.

		unsigned short countLine = 0; // количество байт в упакованной строке
		countOutliers = 0; // счетчик количества выбросов в текущей строке
		j = 0;
		while(j < n) // Находим выбросы
		{
			if(dataLine[j] > thereshold) // Выброс найден - обрабатываем его.
			{
				tmpSignalResponse.height_begin = static_cast<unsigned long>(floor(1.0 * j * ionogram->getHeightStep()));
				tmpSignalResponse.count_samples = 1;
				tmpAmplitude[tmpSignalResponse.count_samples-1] = dataLine[j];
				j++; // переход к следующему элементу
				while(dataLine[j] > thereshold && j < n)
				{
					tmpSignalResponse.count_samples++;
					tmpAmplitude[tmpSignalResponse.count_samples-1] = dataLine[j];
					j++; // переход к следующему элементу
				}
				countOutliers++; // прирастим количество выбросов в строке

				// Собираем упакованные выбросы
				// Заголовок выброса.
				unsigned short nn = sizeof(SignalResponse);
				memcpy(tmpLine+countLine, &tmpSignalResponse, nn);
				countLine += nn;
				// Данные выброса.
				nn = tmpSignalResponse.count_samples*sizeof(unsigned char);
				memcpy(tmpLine+countLine, tmpAmplitude, nn);
				countLine += nn;
			}
			else
				j++; // тупо двигаемся дальше
		}

		// Данные для текущей частоты помещаем в буфер.
		// Заголовки частот пишутся всегда, даже если выбросы отсутствуют.
		tmpFrequencyData.count_o = countOutliers;
		// Заголовок частоты.
		unsigned nFrequencyData = sizeof(FrequencyData);
		memcpy(tmpArray, &tmpFrequencyData, nFrequencyData);
		// сохраняем ранее сформированную цепочку выбросов
		if(countLine)
			memcpy(tmpArray+nFrequencyData, tmpLine, countLine);

		// Заполняем выходную структуру и чистим память.
		setSavedSize(countLine + nFrequencyData);
		_buf_ionogram = new unsigned char [getSavedSize()];
		memcpy(_buf_ionogram, reinterpret_cast<char*>(tmpArray), getSavedSize());

		delete [] tmpLine;
		delete [] tmpAmplitude;
		delete [] tmpArray;
		delete [] dataLine;
	}

	/// <summary>
	/// Подготовка буфера квадратур для сохранения линии в файл.
	/// Объём буфера уменьшен путём отбора точек, амплитуда которых больше уровня ограничения, т.е., предположительно,
	/// соответствует появлению сигнала отражения в отклике.
	/// </summary>
	/// <param name="amplitudes">Ссылка на объект класса, содержащий инициализационную информацию об эксперименте.</param>
	/// <param name="curFrq">Частота зондирования, дБ.</param>
	/// <param name="gain">Усиление приемника, учитывающее аттенюатор, дБ.</param>
	void lineADC::prepareIPG_QuadraturesBuffer(xml_amplitudes& amplitudes, const unsigned short curFrq, const unsigned short gain)
	{
		// Неизменяемые данные по текущей частоте
		FrequencyData tmpFrequencyData;
		tmpFrequencyData.gain_control = gain;// !< Значение ослабления входного аттенюатора дБ.
		tmpFrequencyData.pulse_time = static_cast<unsigned short>(floor(1000./amplitudes.getPulseFrq()));//!< Время зондирования на одной частоте, [мс].
		tmpFrequencyData.pulse_length = static_cast<unsigned char>(amplitudes.getPulseDuration());//!< Длительность зондирующего импульса, [мкc].
		tmpFrequencyData.band = static_cast<unsigned char>(floor(1000000./amplitudes.getPulseDuration()));//!< Полоса сигнала, [кГц].
		tmpFrequencyData.type = 0;														//!< Вид модуляции (0 - гладкий импульс, 1 - ФКМ).

		// Выделим буфер под упакованную строку. Считаем, что упакованная строка не больше исходной.
		unsigned n = getSavedSize();
		unsigned char *tmpArray = new unsigned char [n];
		unsigned char *tmpLine = new unsigned char [n];
		unsigned char *tmpAmplitude = new unsigned char [n];
  
		unsigned j;
		unsigned char *dataLine = new unsigned char [n];
		// Усечение данных до размера 8 бит.
		for(unsigned i = 0; i < n; i++) 
			dataLine[i] = static_cast<unsigned char>(static_cast<unsigned short>(_abs.at(i)) >> 6);

		SignalResponse tmpSignalResponse;
		unsigned char countOutliers;
		unsigned short countFrq = 0; // количество байт в упакованном частотном массиве

		// Определим уровень наличия выбросов.
		unsigned char thereshold = static_cast<unsigned char>(calculateAbsThereshold(dataLine));           
		tmpFrequencyData.frequency = curFrq;
		tmpFrequencyData.threshold_o = thereshold;
		tmpFrequencyData.threshold_x = 0;
		tmpFrequencyData.count_o = 0; // может изменяться
		tmpFrequencyData.count_x = 0; // Антенна у нас одна о- и х- компоненты объединены.
	}
	
	void lineADC::prepareDirty_IonogramBuffer()
	{
		// Усечение данных до размера 8 бит.
		for(size_t i = 0; i < getSavedSize(); i++) 
			_buf_ionogram[i] = static_cast<unsigned char>(static_cast<unsigned short>(_abs.at(i)) >> 6);
	}

	/// <summary>
	/// Prepares the dirty amplitudes buffer. Простое копирование данных. Используется только нижняя половина отсчётов.
	/// </summary>
	void lineADC::prepareDirty_AmplitudesBuffer()
	{
		memcpy(_buf_amplitudes, _buf, sizeof(unsigned long) * getSavedSize());
	}

	/// <summary>
	/// Вычисляет смещение нуля по верхней половине вектора измерений.
	/// </summary>
	/// <param name="vec">Входной вектор.</param>
	/// <returns>Значение смещения нуля.</returns>
	template<typename T>
	double lineADC::calculateZeroShift(const std::vector<T>& vec)
	{
		double _out;
		int n = vec.size();
		std::vector<T> vec_half;
		vec_half.insert(vec_half.begin(),vec.begin()+(n/2-1),vec.end()); // верхняя половина вектора

		// Сортируем по возрастанию и определяем медиану.
		sort(vec_half.begin(), vec_half.end());
		_out = (vec_half.at(n/2),vec_half.at(n/2-1))/2.;

		return _out;
	}

	/// <summary>
	/// Вычисляет пороговый уровень, выше которого, предположительно, регистрируется сигнал отражения.
	/// </summary>
	/// <param name="vec">Входной вектор абсолютных значений амплитуд.</param>
	/// <returns>Значение порога.</returns>
	double lineADC::calculateAbsThereshold(unsigned char* _in)
	{
		double thereshold;
		unsigned n = __COUNT_MAX__;
		std::vector<double> vec;

		// 0. Заполним рабочий вектор.
		for(size_t i = 0; i < n; i++) 
			vec.at(i) = _in[i];
		// 1. Упорядочим данные по возрастанию.
		sort(vec.begin(), vec.end());
		// 2. Квартили (n - чётное, степень двойки!!!)
		double Q1 = (vec.at(n/4)+vec.at(n/4-1))/2.;
		double Q3 = (vec.at(3*n/4),vec.at(3*n/4-1))/2.;
		// 3. Межквартильный диапазон
		double dQ = Q3 - Q1;
		// 4. Верхняя граница выбросов.
		thereshold = Q3 + 3 * dQ;

		return thereshold;
	}

	// *************************************************************************
	// Class CBuffer
	// Обработка буфера АЦП и подготовка для вывода в файл функцией WriteFile.
	// *************************************************************************
	
	/// <summary>
	/// Initializes a new instance of the <see cref="CBuffer"/> class.
	/// По умолчанию:
	///	saved_count_ = __COUNT_MAX__/2 (число точек, сохраняемых в файл),
	/// Все массивы заполняются нулями. Размер массивов = __COUNT_MAX__.
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
	/// <param name="obj">Объект класса CBuffer.</param>
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
	/// <param name="adc">Указатель на исходный буфер АЦП.</param>
	/// <param name="count">Число точек (32-бит) в исходном буфере.</param>
	/// <param name="saved_count">Число точек для сохранения.</param>
	/// По умолчанию:
	/// count_ = __COUNT_MAX__ (число точек во входном буфере),
	///	saved_count_ = __COUNT_MAX__/2 (число точек, сохраняемых в файл).
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
		if(ArrayToFile_)
			delete [] ArrayToFile_;
	}

	/// <summary>
	/// Аккумулирует указанный буфер АЦП в объекте. Количество точек уже
	/// установлено в count_.
	/// </summary>
	/// <param name="adc">Указатель на буфер АЦП.</param>
	void CBuffer::accumulate(BYTE* adc)
	{
		unsigned long *_tmp;
		_tmp = new unsigned long [re_.size()];
		// В буффере АЦП сохранены два сырых 16-битных чередующихся канала на 
		// одной частоте (count - количество 32-разрядных слов).
		memcpy(_tmp, adc, re_.size()*4); // копируем весь аппаратный буфер

		for(size_t i = 0; i < re_.size(); i++)
		{
	        // Используем двухканальную интерпретацию через анонимную структуру
	        union {
	            unsigned long word;     // 4-байтное слово двухканального АЦП
	            adcTwoChannels twoCh;  // двухканальные (квадратурные) данные
	        };
	            
			// Разбиение на квадратуры. Значимы только старшие 14 бит.
	        word = _tmp[i];
			re_.at(i) = twoCh.re.value; // уже смещено на 2 байта вправо
	        im_.at(i) = twoCh.im.value; // уже смещено на 2 байта вправо
			abs_.at(i) += sqrt(
				pow(static_cast<double>(re_.at(i)),2) + 
				pow(static_cast<double>(im_.at(i)),2)
				);
		
			// Выбросим исключение для обработки нештатной ситуации, чтобы 
			// уменьшить коэффициент усиления. В верхней процедуре предусмотреть 
			// заполнение LOGa частоты, на которой произошло исключение.
			// !!! Данные могут заполниться не полностью !!!
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
		if (this == &obj) // проверка на себяприсваиваивание
			return *this;

		saved_count_ = obj.getSavedSize();
		re_.assign(obj.re_.begin(),obj.re_.end());
		im_.assign(obj.im_.begin(),obj.im_.end());
		abs_.assign(obj.abs_.begin(),obj.abs_.end());

		return *this; // возвращаем ссылку на текущий объект
	}
	
	/// <summary>
	/// Operator+= аккумулирует абсолютные амплитуды сигнала.
	/// Сохраняются последние квадратурные амплитуды.
	/// </summary>
	/// <param name="adc">Сырой массив отсчётов АЦП.</param>
	/// <returns></returns>
	CBuffer& CBuffer::operator+=(BYTE* adc)
	{
		accumulate(adc);
		return *this; // возвращаем ссылку на текущий объект
	}
	
	/// <summary>
	/// Operator/= производит деление накопленного вектора абсолютных амплитуд
	/// на число в интервале -127..+127.
	/// </summary>
	/// <param name="value">Делитель типа char.</param>
	/// <returns></returns>
	CBuffer& CBuffer::operator/=(char value)
	{
		if(value)
			for (auto it = abs_.begin(); it != abs_.end(); ++it)
				*it /= value;
		return *this; // возвращаем ссылку на текущий объект
	}

	/// <summary>
	/// Возвращает смещение нуля для вектора произвольного типа.
	/// Используем только верхнюю половину вектора, где отражения малы.
	/// </summary>
	/// <param name="vec">Вектор произвольного типа.</param>
	/// <returns>Тип double.</returns>
	template<typename T>
	double const CBuffer::calculate_zero_shift(const std::vector<T>& vec) const
	{
		double tmp;

		int n = vec.size()/2;
		std::vector<short> top_half;
		top_half.insert(top_half.begin(),vec.begin()+n,vec.end()); 

		// Сортируем по возрастанию и определяем медиану.
		sort(top_half.begin(), top_half.end());
		tmp = (top_half.at(n/2) + top_half.at(n/2-1))/2.;

		return tmp;
	}

	template<typename T>
	double const CBuffer::calculate_thereshold(const T* _in, size_t count) const
	{
		double thereshold;
		std::vector<double> vec(count,0);

		// 0. Заполним рабочий вектор.
		for(size_t i = 0; i < count; i++) 
			vec.at(i) = _in[i];
		// 1. Упорядочим данные по возрастанию.
		sort(vec.begin(), vec.end());
		// 2. Квартили (n - чётное, степень двойки!!!)
		double Q1 = (vec.at(count/4)+vec.at(count/4-1))/2.;
		double Q3 = (vec.at(3*count/4),vec.at(3*count/4-1))/2.;
		// 3. Межквартильный диапазон
		double dQ = Q3 - Q1;
		// 4. Верхняя граница выбросов.
		thereshold = Q3 + 3 * dQ;

		return thereshold;
	}

	/// <summary>
	/// Возвращает смещение нуля для обоих каналов.
	/// </summary>
	/// <returns>Тип CPoint.</returns>
	CPoint CBuffer::getZeroShift() const
	{
		CPoint out;
		out.re = static_cast<short>(calculate_zero_shift(re_));
		out.im = static_cast<short>(calculate_zero_shift(im_));

		return out;
	}

	/// <summary>
	/// Подготовка черновой ионограммы без усечения данных для записи в файл.
	/// Заполнение private свойств:
	/// BYTE* ArrayToFile_;
	/// size_t BytesCountToFile_;
	/// </summary>
	void CBuffer::prepareIonogram_Dirty()
	{
		// Проверим на существование и уничтожим, если распределялся.
		if(ArrayToFile_)
			delete [] ArrayToFile_;

		unsigned short *dataLine = new unsigned short [getSavedSize()];

		// Усечение данных до размера 8 бит.
		for(size_t i = 0; i < getSavedSize(); i++) 
			dataLine[i] = static_cast<unsigned short>(abs_.at(i));
		BytesCountToFile_ = 2 * getSavedSize(); // записываем short числа

		ArrayToFile_ = new BYTE [BytesCountToFile_];
		memcpy(ArrayToFile_, reinterpret_cast<BYTE*>(dataLine), BytesCountToFile_);

		delete [] dataLine;
	}

	/// <summary>
	/// Подготовка ионограммы в формате ИПГ для записи в файл.
	/// Заполнение private свойств:
	/// BYTE* ArrayToFile_;
	/// size_t BytesCountToFile_;
	/// </summary>
	/// <param name="ionogram">const xml_ionogram& - параметры зондирования для ионограммы.</param>
	/// <param name="curFrq">const unsigned short - текущая частота зондирования, кГц.</param>
	void CBuffer::prepareIonogram_IPG(const xml_ionogram& ionogram, const unsigned short curFrq)
	{
		try{
		// Неизменяемые данные по текущей частоте
		FrequencyData tmpFrequencyData;
		tmpFrequencyData.gain_control = static_cast<unsigned short>(ionogram.getGain());// !< Значение ослабления входного аттенюатора дБ.
		tmpFrequencyData.pulse_time = static_cast<unsigned short>(floor(1000./ionogram.getPulseFrq()));//!< Время зондирования на одной частоте, [мс].
		tmpFrequencyData.pulse_length = static_cast<unsigned char>(ionogram.getPulseDuration());//!< Длительность зондирующего импульса, [мкc].
		tmpFrequencyData.band = static_cast<unsigned char>(floor(1000000./ionogram.getPulseDuration()));//!< Полоса сигнала, [кГц].
		tmpFrequencyData.type = 0;														//!< Вид модуляции (0 - гладкий импульс, 1 - ФКМ).

		unsigned j;
		unsigned n = getSavedSize();
		unsigned char *tmpArray;
		unsigned char *tmpLine;
		unsigned char *tmpAmplitude;
		unsigned char *dataLine;

		// Выделим буфер под упакованную строку. Считаем, что упакованная строка не больше исходной.
		try{
		tmpArray = new unsigned char [n];
		tmpLine = new unsigned char [n];
		tmpAmplitude = new unsigned char [n];
		dataLine = new unsigned char [n];
						}
				catch(const std::bad_alloc& e) // Отлавливаем здесь только ошибки ограничения амплитуды.
				{
					std::cout << e.what();
				}

		// Усечение данных до размера 8 бит.
		for(unsigned i = 0; i < n; i++) 
			dataLine[i] = static_cast<unsigned char>(abs_.at(i)/64);

		SignalResponse tmpSignalResponse;
		unsigned char countOutliers;
		unsigned short countFrq = 0; // количество байт в упакованном частотном массиве

		// Определим уровень наличия выбросов.
		unsigned char thereshold = static_cast<unsigned char>
			(floor(0.5 + calculate_thereshold(dataLine, n))); // отсутствует round!!!
		tmpFrequencyData.frequency = curFrq;
		tmpFrequencyData.threshold_o = thereshold;
		tmpFrequencyData.threshold_x = 0;
		tmpFrequencyData.count_o = 0; // может изменяться
		tmpFrequencyData.count_x = 0; // Антенна у нас одна о- и х- компоненты объединены.

		unsigned short countLine = 0; // количество байт в упакованной строке
		countOutliers = 0; // счетчик количества выбросов в текущей строке
		j = 0;

		while(j < n) // Находим выбросы
		{
			if(dataLine[j] > thereshold) // Выброс найден - обрабатываем его.
			{
				tmpSignalResponse.height_begin = static_cast<unsigned long>(floor(1.0 * j * ionogram.getHeightStep()));
				tmpSignalResponse.count_samples = 1;
				tmpAmplitude[tmpSignalResponse.count_samples-1] = dataLine[j];
				j++; // переход к следующему элементу
				while(dataLine[j] > thereshold && j < n)
				{
					tmpSignalResponse.count_samples++;
					tmpAmplitude[tmpSignalResponse.count_samples-1] = dataLine[j];
					j++; // переход к следующему элементу
				}
				countOutliers++; // прирастим количество выбросов в строке

				// Собираем упакованные выбросы
				// Заголовок выброса.
				unsigned short nn = sizeof(SignalResponse);
				memcpy(tmpLine+countLine, &tmpSignalResponse, nn);
				countLine += nn;
				// Данные выброса.
				nn = tmpSignalResponse.count_samples*sizeof(unsigned char);
				memcpy(tmpLine+countLine, tmpAmplitude, nn);
				countLine += nn;
			}
			else
				j++; // тупо двигаемся дальше
		}

		// Данные для текущей частоты помещаем в буфер.
		// Заголовки частот пишутся всегда, даже если выбросы отсутствуют.
		tmpFrequencyData.count_o = countOutliers;
		// Заголовок частоты.
		unsigned nFrequencyData = sizeof(FrequencyData);
		memcpy(tmpArray, &tmpFrequencyData, nFrequencyData);
		// сохраняем ранее сформированную цепочку выбросов
		if(countLine)
			memcpy(tmpArray+nFrequencyData, tmpLine, countLine);

		// Проверим на существование и уничтожим, если распределялся.
		if(ArrayToFile_)
			delete [] ArrayToFile_;
		// Заполним хранилище данными.
		BytesCountToFile_ = countLine + nFrequencyData;
		ArrayToFile_ = new BYTE [BytesCountToFile_];
		memcpy(ArrayToFile_, reinterpret_cast<BYTE*>(tmpArray), BytesCountToFile_);

		delete [] tmpLine;
		delete [] tmpAmplitude;
		delete [] tmpArray;
		delete [] dataLine;

								}
				catch(const std::bad_alloc& e) // Отлавливаем здесь только ошибки ограничения амплитуды.
				{
					std::cout << e.what();
				}
	}

	void CBuffer::prepareAmplitudes_Dirty()
	{

	}

	void CBuffer::prepareAmplitudes_IPG()
	{

	}

	// *************************************************************************
	// Конец Class CBuffer
	// *************************************************************************

} // namespace parus

	//// Возвращает статистику строки данных (short/double).
	//template<typename T>
	//Statistics calculateStatistics(const std::vector<T>& vec)
	//{
	//	Statistics _out;
	//	int n = vec.size();

	//	std::vector<T> vec_half;
	//	vec_half.insert(vec_half.begin(),vec.begin()+(n/2-1),vec.end()); // верхняя половина вектора

	//	// 1. Упорядочим данные по возрастанию.
	//	sort(vec.begin(), vec.end());
	//	sort(vec_half.begin(), vec_half.end());
	//	// 2. Квартили (n - чётное, степень двойки!!!)
	//	_out.Q1 = (vec.at(n/4)+vec.at(n/4-1))/2.;
	//	_out.median = (vec.at(n/2),vec.at(n/2-1))/2.;
	//	_out.median_top_half = (vec_half.at(n/2),vec_half.at(n/2-1))/2.;
	//	_out.Q3 = (vec.at(3*n/4),vec.at(3*n/4-1))/2.;
	//	// 3. Межквартильный диапазон
	//	_out.dQ = _out.Q3 - _out.Q1;
	//	// 4. Верхняя граница выбросов.
	//	_out.thereshold = _out.Q3 + 3 * _out.dQ;

	//	// среднее значение
	//	_out.mean = std::accumulate(vec.begin(), vec.end(), 0) / n;

	//	// стандартное отклонение от среднего значения
	//	std::vector<double> diff(n);
	//	std::transform(vec.begin(), vec.end(), diff.begin(),
 //              std::bind2nd(std::minus<double>(), _out.mean));
	//	double sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
	//	_out.std_mean = std::sqrt(sq_sum /(n-1));

	//	// стандартное отклонение от медианы
	//	std::vector<double> diff(n);
	//	std::transform(vec.begin(), vec.end(), diff.begin(),
 //              std::bind2nd(std::minus<double>(), _out.median));
	//	double sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
	//	_out.std_median = std::sqrt(sq_sum /(n-1));

	//	// минимальное и максимальное значения
	//	_out.min = vec.front();
	//	_out.max = vec.back();

	//	return _out;
	//}