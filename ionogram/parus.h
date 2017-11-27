// ===========================================================================
// Заголовочный файл для работы с аппаратурой.
// ===========================================================================
#ifndef __PARUS_H__
#define __PARUS_H__

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// Помним, что структуры в памяти выравниваются в зависимости
// от компилятора, разрядности и архитектуры системы. 
// Оптимальный выход в использовании машинно независимых форматов
// данных. Например HDF, CDF и.т.п.
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#include <iomanip>
#include <string>
#include <sstream>
#include <stdexcept>
#include <conio.h>

// Для ускорения сохранения в файл используются функции Windows API.
#include <windows.h>

// Заголовочные файлы для работы с модулем АЦП.
// Включение в проект библиотеки импорта daqdrv.dll
#include "daqdef.h"
#pragma comment(lib, "daqdrv.lib") // for Microsoft Visual C++
#include "m14x3mDf.h"

// Заголовочные файлы для работы с данными.
#include "config.h"

// 14 бит - без знака, 13 бит по модулю со знаком для каждой квадратуры = 8191.
// sqrt(8191^2 + 8191^2) = 11583... Берем 11580.
#define __AMPLITUDE_MAX__ 11580 // константа, определяющая максимум амплитуды, выше которого подозреваем ограничение сигнала

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

	// Класс работы с аппаратурой Паруса.
	class parusWork {

		HANDLE _hFile; // выходной файл данных

		unsigned int _g;
		char _att;
		unsigned int _fsync;
		unsigned int _pulse_duration;
		unsigned int _curFrq;
    
		unsigned _height_step; // шаг по высоте, м
		unsigned _height_count; // количество высот
		unsigned _height_min; // начальная высота, км (всё, что ниже при обработке отбрасывается)

		int _RetStatus;
		M214x3M_DRVPARS _DrvPars;
		int _DAQ; // дескриптор устройства (если равен нулю, то значит произошла ошибка)
		DAQ_ASYNCREQDATA *_ReqData; // цепочка буферов АЦП
		DAQ_ASYNCXFERDATA _xferData; // параметры асинхронного режима
		DAQ_ASYNCXFERSTATE _curState;

		unsigned long *_fullBuf; // грязная строка данных
		unsigned long buf_size;

		BYTE *getBuffer(void){return (BYTE*)(_ReqData->Tbl[0].Addr);}
		DWORD getBufferSize(void){return (DWORD)(_ReqData->Tbl[0].Size);}

		HANDLE _LPTPort; // для конфигурирования синтезатора
		HANDLE initCOM2(void);
		void initLPT1(void);

		void closeOutputFile(void);

		// Работа с ионограммами
		unsigned short *_sum_abs; // массив абсолютных значений

		// Работа с файлом журнала
		std::vector<std::string> _log;

	public:
		// Глобальные описания
		static const std::string _PLDFileName;
		static const std::string _DriverName;
		static const std::string _DeviceName;
		static const double _C; // скорость света в вакууме

		parusWork(xmlconfig* conf);
		~parusWork(void);

		M214x3M_DRVPARS initADC(unsigned int nHeights);

		void ASYNC_TRANSFER(void);
		int READ_BUFISCOMPLETE(unsigned long msTimeout);
		void READ_ABORTIO(void);
		void READ_GETIOSTATE(void);
		int READ_ISCOMPLETE(unsigned long msTimeout);

		void adjustSounding(unsigned int curFrq);
		void startGenerator(unsigned int nPulses);

		// Работа с ионограммами
		void openIonogramFile(xmlconfig* conf);
		void cleanLineAccumulator(void);
		void accumulateLine(unsigned short curFrq); // суммирование по импульсам на одной частоте
		void averageLine(unsigned pulse_count); // усреднение по импульсам на одной частоте
		unsigned char getThereshold(unsigned char *arr, unsigned n);
		void saveLine(unsigned short curFrq); // Усечение данных до char (сдвиг на 6 бит) и сохранение линии в файле.
		void saveDirtyLine(void);

		// Работа с файлами выходных данных
		void openDataFile(xmlconfig* conf);
		void saveFullData(void);
		void saveDataWithGain(void);

		// Работа с файлом журнала
		std::vector<std::string> getLog(void){ return _log;}
	};

	int comp(const void *i, const void *j);

} // namespace parus

#endif // __PARUS_H__