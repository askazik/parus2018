#include "ParusLine.h"

namespace parus {

	// Class lineADC
	// Инициализация без заполнения.
	lineADC::lineADC() :
		_buf(nullptr),
		_saved_buf_size(__COUNT_MAX__)
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
		fill(buf);
	}

	// Чистим буферы данных
	lineADC::~lineADC()
	{
		if(_buf) delete [] _buf;
	}

	// Заполнение обрабатываемых векторов из аппаратного буфера.
	bool lineADC::fill(BYTE *buf)
	{
		// Признак ограничения сигнала.
		bool isLimitExceeded = false;

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
			_re[i] = twoCh.re.value>>2;
	        _im[i] = twoCh.im.value>>2;
			// Амплитуды суммируем с находящимися в хранилище.
			_abs[i] += static_cast<short>(sqrt(pow(static_cast<double>(_re.at(i)),2) + pow(static_cast<double>(_im.at(i)),2)));

			if(abs(_re[i]) >= __AMPLITUDE_MAX__ || abs(_im[i]) >= __AMPLITUDE_MAX__)
			{
				isLimitExceeded = true;
				// Выбросим исключение для обработки нештатной ситуации, чтобы уменьшить коэффициент усиления.
				// В верхней процедуре предусмотреть заполнение частоты, на которой произошло исключение.
				throw std::range_error("Ограничение сигнала.");
			}
		}

		return isLimitExceeded;
	}

	// Возвращает статистику строки данных.
	Statistics lineADC::calculateStatistics(std::vector<short> vec)
	{
		Statistics _out;
		int n = vec.size();

		// 1. Упорядочим данные по возрастанию.
		sort(vec.begin(), vec.end());
		// 2. Квартили (n - чётное, степень двойки!!!)
		_out.Q1 = (vec.at(n/4)+vec.at(n/4-1))/2.;
		_out.median = (vec.at(n/2),vec.at(n/2-1))/2.;
		_out.Q3 = (vec.at(3*n/4),vec.at(3*n/4-1))/2.;
		// 3. Межквартильный диапазон
		_out.dQ = _out.Q3 - _out.Q1;
		// 4. Верхняя граница выбросов.
		_out.maxLim = _out.Q3 + 3 * _out.dQ;

		// среднее значение
		_out.mean = accumulate(vec.begin(), vec.end(), 0) / n;

		// стандартное отклонение
		std::vector<double> diff(n);
		std::transform(vec.begin(), vec.end(), diff.begin(),
               std::bind2nd(std::minus<double>(), _out.mean));
		double sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
		_out.std = std::sqrt(sq_sum /(n-1));

		// минимальное и максимальное значения
		_out.min = vec.front();
		_out.max = vec.back();

		return _out;
	}

	// Усечение данных до char (сдвиг на 6 бит) и сохранение линии в файле.
	// Упаковка строки ионограммы по алгоритму ИЗМИРАН-ИПГ.
	// data - сырая строка ионограммы
	// conf - кофигурация зондирования
	// numModule - номер модуля зондирования
	// curFrq - частота зондирования, кГц
	IPGLine lineADC::getIPGBufer(void)
	{
		IPGLine _out;

		// Неизменяемые данные по текущей частоте
		FrequencyData tmpFrequencyData;
		//tmpFrequencyData.gain_control = static_cast<unsigned short>(_g * 6);			// !< Значение ослабления входного аттенюатора дБ.
		//tmpFrequencyData.pulse_time = static_cast<unsigned short>(floor(1000./_fsync));	//!< Время зондирования на одной частоте, [мс].
		//tmpFrequencyData.pulse_length = static_cast<unsigned char>(_pulse_duration);	//!< Длительность зондирующего импульса, [мкc].
		//tmpFrequencyData.band = static_cast<unsigned char>(floor(1000000./_pulse_duration)); //!< Полоса сигнала, [кГц].
		//tmpFrequencyData.type = 0;														//!< Вид модуляции (0 - гладкий импульс, 1 - ФКМ).

		//// Выделим буфер под упакованную строку. Считаем, что упакованная строка не больше исходной.
		//unsigned n = _height_count;
		//unsigned char *tmpArray = new unsigned char [n];
		//unsigned char *tmpLine = new unsigned char [n];
		//unsigned char *tmpAmplitude = new unsigned char [n];
  //  
		//unsigned j;
		//unsigned char *dataLine = new unsigned char [n];

		//// Усечение данных до размера 8 бит.
		//for(unsigned i = 0; i < n; i++) 
		//	dataLine[i] = static_cast<unsigned char>(_sum_abs[i] >> 6);

		//unsigned char thereshold;
		//SignalResponse tmpSignalResponse;
		//unsigned char countOutliers;
		//unsigned short countFrq = 0; // количество байт в упакованном частотном массиве

		//// Определим уровень наличия выбросов.
		//thereshold = getThereshold(dataLine, n);            
		//tmpFrequencyData.frequency = curFrq;
		//tmpFrequencyData.threshold_o = thereshold;
		//tmpFrequencyData.threshold_x = 0;
		//tmpFrequencyData.count_o = 0; // может изменяться
		//tmpFrequencyData.count_x = 0; // Антенна у нас одна о- и х- компоненты объединены.

		//unsigned short countLine = 0; // количество байт в упакованной строке
		//countOutliers = 0; // счетчик количества выбросов в текущей строке
		//j = 0;
		//while(j < n) // Находим выбросы
		//{
		//	if(dataLine[j] > thereshold) // Выброс найден - обрабатываем его.
		//	{
		//		tmpSignalResponse.height_begin = static_cast<unsigned long>(floor(1.0 * j * _height_step));
		//		tmpSignalResponse.count_samples = 1;
		//		tmpAmplitude[tmpSignalResponse.count_samples-1] = dataLine[j];
		//		j++; // переход к следующему элементу
		//		while(dataLine[j] > thereshold && j < n)
		//		{
		//			tmpSignalResponse.count_samples++;
		//			tmpAmplitude[tmpSignalResponse.count_samples-1] = dataLine[j];
		//			j++; // переход к следующему элементу
		//		}
		//		countOutliers++; // прирастим количество выбросов в строке

		//		// Собираем упакованные выбросы
		//		// Заголовок выброса.
		//		unsigned short nn = sizeof(SignalResponse);
		//		memcpy(tmpLine+countLine, &tmpSignalResponse, nn);
		//		countLine += nn;
		//		// Данные выброса.
		//		nn = tmpSignalResponse.count_samples*sizeof(unsigned char);
		//		memcpy(tmpLine+countLine, tmpAmplitude, nn);
		//		countLine += nn;
		//	}
		//	else
		//		j++; // тупо двигаемся дальше
		//}

		//// Данные для текущей частоты помещаем в буфер.
		//// Заголовки частот пишутся всегда, даже если выбросы отсутствуют.
		//tmpFrequencyData.count_o = countOutliers;
		//// Заголовок частоты.
		//unsigned nFrequencyData = sizeof(FrequencyData);
		//memcpy(tmpArray, &tmpFrequencyData, nFrequencyData);
		//// сохраняем ранее сформированную цепочку выбросов
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
		//	throw std::runtime_error("Не могу сохранить строку ионограммы в файл.");
	
		//delete [] tmpLine;
		//delete [] tmpAmplitude;
		//delete [] tmpArray;
		//delete [] dataLine;

		return _out;
	} 

} // namespace parus