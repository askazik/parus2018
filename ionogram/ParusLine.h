// ===========================================================================
// Заголовочный файл для работы с линией АЦП.
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

// 14 бит - без знака, 13 бит по модулю со знаком для каждой квадратуры = 8191.
#define __AMPLITUDE_MAX__ 8190 // константа, определяющая максимум амплитуды, выше которого подозреваем ограничение сигнала
// Число высот (размер исходного буфера АЦП при зондировании, fifo нашего АЦП 4Кб. Т.е. не больше 1024 отсчётов для каждого из двух квадратурных каналов).
#define __COUNT_MAX__ 1024 // константа, определяющая максимуальное число единовременно измеряемых спаренных квадратурных отсчётов

#endif // __LINEADC_H__

namespace parus {

	#pragma pack(push, 1)

	struct adcChannel {
		unsigned short b0: 1;  // Устанавливается в 1 в первом отсчете блока для каждого канала АЦП (т.е. после старта АЦП или после каждого старта в старт-стопном режиме).
		unsigned short b1: 1;  // Устанавливается в 1 в отсчете, который соответствует последнему опрашиваемому входу (по порядку расположения номеров входов в памяти мультиплексора) для каждого канала АЦП.    
		short value : 14; // данные из одного канала АЦП, 14-разрядное слово
	};
	
	struct adcTwoChannels {
		adcChannel re; // условно первая квадратура
		adcChannel im; // условно вторая квадратура
	};

	struct ionPackedData { // Упакованные данные ионограммы.
		unsigned size; // Размер упакованной ионограммы в байтах.
		unsigned char *ptr;   // Указатель на блок данных упакованной ионограммы.
	};

	/* =====================================================================  */
	/* Родные структуры данных ИПГ-ИЗМИРАН */
	/* =====================================================================  */
	/* Каждая строка начинается с заголовка следующей структуры */
	struct FrequencyData {
		unsigned short frequency; //!< Частота зондирования, [кГц].
		unsigned short gain_control; // !< Значение ослабления входного аттенюатора дБ.
		unsigned short pulse_time; //!< Время зондирования на одной частоте, [мс].
		unsigned char pulse_length; //!< Длительность зондирующего импульса, [мкc].
		unsigned char band; //!< Полоса сигнала, [кГц].
		unsigned char type; //!< Вид модуляции (0 - гладкий импульс, 1 - ФКМ).
		unsigned char threshold_o; //!< Порог амплитуды обыкновенной волны, ниже которого отклики не будут записываться в файл, [Дб/ед. АЦП].
		unsigned char threshold_x; //!< Порог амплитуды необыкновенной волны, ниже которого отклики не будут записываться в файл, [Дб/ед. АЦП].
		unsigned char count_o; //!< Число сигналов компоненты O.
		unsigned char count_x; //!< Число сигналов компоненты X.
	};

	/* Сначала следуют FrequencyData::count_o структур SignalResponse, описывающих обыкновенную волну. */
	/* Cразу же после перечисления всех SignalResponse  и SignalSample  для обыкновенных откликов следуют FrequencyData::count_x */
	/* структур SignalResponse, описывающих необыкновенные отклики с массивом структур SignalSample после каждой из них. Величины */
	/* FrequencyData::count_o и FrequencyData::count_x могут быть равны нулю, тогда соответствующие данные отсутствуют. */
	struct SignalResponse {
		unsigned long height_begin; //!< начальная высота, [м]
		unsigned short count_samples; //!< Число дискретов
	};

	/* =====================================================================  */
	#pragma pack(pop)

	// Структура, хранящая характеристики выборки.
	struct CStatistics
	{
		double Q1, Q3, dQ; // квартили и межквартильный диапазон
		double thereshold; // верхняя граница выбросов
		double median; // медиана
		double median_top_half; // медиана верхней половины выборки - для определения смещения 0
		double mean; // среднее значение
		double std_mean; // стандартное отклонение (корень из дисперсии) от среднего
		double std_median; // стандартное отклонение (корень из дисперсии) от медианы
		double min, max;
	};
	
	struct CPoint {
		short re; // условно первая квадратура
		short im; // условно вторая квадратура
	};

	// =========================================================================
	// Новый класс для работы с строкой АЦП. 28.12.2017.
	class CBuffer
	{
		// количество точек данных (начиная с нулевого уровня высоты), 
		// подготавливаемых для сохранения в файл
		unsigned saved_count_;

		// данные
		std::vector<short> re_, im_; // 13 бит со знаком
		std::vector<double> abs_;

		BYTE* ArrayToFile_;
		size_t BytesCountToFile_;
	
		void accumulate(BYTE* adc);
		template<typename T>
		double calculate_zero_shift(const std::vector<T>& vec) const;
		template<typename T>
		double calculate_thereshold(<T>* arr, size_t count) const;
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

		// подготовка массива к записи в файл
		size_t getBytesCountToFile() const {return BytesCountToFile_;}
		BYTE* getBytesArrayToFile() const {return ArrayToFile_;}

		void prepareIonogram_Dirty();
		void prepareIonogram_IPG(const xml_ionogram& ionogram, const unsigned short curFrq);
		void prepareAmplitudes_Dirty();
		void prepareAmplitudes_IPG();
	};
	// =========================================================================

	// Обработка строки, измеренной АЦП.
	class lineADC
	{
		std::vector<int> _re, _im;
		std::vector<double> _abs;
		unsigned long *_buf; // указатель на копию аппаратного буфера
		unsigned char *_buf_ionogram; // указатель на буфер строки ионограммы
		unsigned long *_buf_amplitudes; // указатель на буфер строки амплитуд

		size_t _real_buf_size; // размер буфера для сохранения результатов (уменьшаем размер выходного файла)
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
		const size_t getSavedSize(void){return _real_buf_size;}
		const unsigned long* getBufer(void){return _buf;}
		
		void prepareIPG_IonogramBuffer(xml_ionogram* ionogram, const unsigned short curFrq);
		void prepareIPG_QuadraturesBuffer(xml_amplitudes& amplitudes, const unsigned short curFrq, const unsigned short gain);
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