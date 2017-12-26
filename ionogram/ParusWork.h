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

// Заголовочные файлы для работы с данными.
#include "ParusConfig.h"
#include "ParusLine.h"

// Для ускорения сохранения в файл используются функции Windows API.
#include <windows.h>

#include <iomanip>
#include <string>
#include <sstream>
#include <stdexcept>
#include <climits>
#include <conio.h>

// Заголовочные файлы для работы с модулем АЦП.
// Включение в проект библиотеки импорта daqdrv.dll
#include "daqdef.h"
#pragma comment(lib, "daqdrv.lib") // for Microsoft Visual C++
#include "m14x3mDf.h"

namespace parus {
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

		void saveLine(char* buf, const size_t byte_counts, const unsigned int curFrq); // сохранение линии в файле
		void saveGain(); // сохранение информации об усилении приёмника
		
		void saveFullData(void);
		void saveDataWithGain(void);

		// Работа с файлом журнала
		std::vector<std::string> _log;

	public:
		// Глобальные описания
		static const std::string _PLDFileName;
		static const std::string _DriverName;
		static const std::string _DeviceName;
		static const double _C; // скорость света в вакууме

		parusWork(void);
		~parusWork(void);

		unsigned int get_g(void){return _g;}
		unsigned int get_fsync(void){return _fsync;}
		unsigned int get_pulse_duration(void){return _pulse_duration;}
		unsigned int get_height_count(void){return _height_count;}
		unsigned int get_height_step(void){return _height_step;}

		M214x3M_DRVPARS initADC(unsigned int nHeights);

		void ASYNC_TRANSFER(void);
		int READ_BUFISCOMPLETE(unsigned long msTimeout);
		void READ_ABORTIO(void);
		void READ_GETIOSTATE(void);
		int READ_ISCOMPLETE(unsigned long msTimeout);

		int GETGAIN(M214x3M_GAINPARS _GAIN); // Получить коэффициент усиления
		int GETINPUTOSV(M214x3M_ZEROSHIFTPARS _ZEROSHIFT); // Получить смещение нуля
		int SETINPUTOSV(M214x3M_ZEROSHIFTPARS _ZEROSHIFT); // Установить смещение нуля
		int GETINP(M214x3M_INPPARS _INPPARS); // Получить включенные входы и их усиление

		void setup(xml_unit* conf);
		int ionogram(xml_unit* conf);
		int amplitudes(xml_unit* conf, unsigned dt);
		void adjustSounding(unsigned int curFrq);
		void adjustZeroShift(std::vector<double> zero_shift);
		void startGenerator(unsigned int nPulses);
		
		void openIonogramFile(xml_ionogram* conf); // Работа с ионограммами
		void openDataFile(xml_amplitudes* conf); // Работа с файлами выходных данных
		void closeOutputFile(void);

		// Работа с файлом журнала
		std::vector<std::string> getLog(void){ return _log;}
	};

	void mySetPriorityClass(void);

} // namespace parus

#endif // __PARUS_H__