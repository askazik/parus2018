#pragma once
// ===========================================================================
// Заголовочный файл для работы с линией АЦП.
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


// 14 бит - без знака, 13 бит по модулю со знаком для каждой квадратуры = 8191.
#define __AMPLITUDE_MAX__ 8190 // константа, определяющая максимум амплитуды, выше которого подозреваем ограничение сигнала
// Число высот (размер исходного буфера АЦП при зондировании, fifo нашего АЦП 4Кб. Т.е. не больше 1024 отсчётов для каждого из двух квадратурных каналов).
#define __COUNT_MAX__ 1024 // константа, определяющая максимуальное число единовременно измеряемых спаренных квадратурных отсчётов

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
	struct Statistics
	{
		double Q1, Q3, dQ; // квартили и межквартильный диапазон
		double maxLim; // верхняя граница выбросов
		double median; // медиана
		double mean; // среднее значение
		double std; // стандартное отклонение (корень из дисперсии)
		double min, max;
	};

	struct IPGLine
	{
		unsigned count; // число байт в массиве для записи строки
		unsigned char *arr; // указатель на массив подготовленных для записи в файл данных

		IPGLine(void) : count(__COUNT_MAX__) {arr = new unsigned char [count];}
		~IPGLine(void){delete [] arr;}
	};

	// Обработка строки, измеренной АЦП.
	class lineADC
	{
		std::vector<short> _re, _im, _abs;
		unsigned long *_buf; // указатель на копию аппаратного буфера
		short _saved_buf_size; // размер буфера для сохранения результатов (уменьшаем размер выходного файла)
	public:
		lineADC();
		lineADC(unsigned long *buf);
		~lineADC();

		bool fill(unsigned long *buf);
		void setSavedSize(short size){_saved_buf_size = size;}
		short getSavedSize(void){return _saved_buf_size;}
		unsigned long * getBufer(void){return _buf;}
		IPGLine getIPGBufer(void);

		Statistics calculateStatistics(std::vector<short> vec);
	};

} // namespace parus

#endif // __LINEADC_H__