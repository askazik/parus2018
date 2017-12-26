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
		unsigned n = __COUNT_MAX__;
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