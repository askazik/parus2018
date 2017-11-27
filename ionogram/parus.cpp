// ====================================================================================
// Класс работы с аппаратурой
// ====================================================================================
#include "parus.h"

namespace parus {

	const std::string parusWork::_PLDFileName = "p14x3m31.hex";
	const std::string parusWork::_DriverName = "m14x3m.dll";
	const std::string parusWork::_DeviceName = "ADM214x3M on AMBPCI";
	const double parusWork::_C = 2.9979e8; // скорость света в вакууме

	parusWork::parusWork(xmlconfig* conf) :
		_hFile(NULL)
	{
		_g = conf->getGain()/6;	// ??? 6дБ = приращение в 4 раза по мощности
		if(_g > 7) _g = 7;

		_att = static_cast<char>(conf->getAttenuation());
		_fsync = conf->getPulseFrq();
		_pulse_duration = conf->getPulseDuration();
		_height_count = conf->getHeightCount(); // количество высот (реально измеренных)
		_height_min = 0; // начальная высота, м

		// Проверка на корректность параметров зондирования.
		// Количество отсчетов = размер буфера FIFO АЦП в 32-разрядных словах.
		std::string stmp = std::to_string(static_cast<unsigned long long>(_height_count));
		if(_height_count < 64 && _height_count >= 4096) // 4096 - сбоит!!! 16К = 4096*4б
			throw std::out_of_range("Размер буфера АЦП должен быть больше 64 и меньше 4096. <" + stmp + ">");
		else
			if(_height_count & (_height_count - 1))
				throw std::out_of_range("Размер буфера АЦП должен быть степенью 2. <" + stmp + ">");

		// Открываем устройство, используя глобальные переменные, установленные заранее.
		_DrvPars = initADC(_height_count);
		_DAQ = DAQ_open(_DriverName.c_str(), &_DrvPars); // NULL 
		if(!_DAQ)
			throw std::runtime_error("Не могу открыть модуль сбора данных.");

		// Запрос памяти для работы АЦП.
		// Два сырых 16-битных чередующихся канала на одной частоте (count - количество 32-разрядных слов).
		buf_size = static_cast<unsigned long>(_height_count * sizeof(unsigned long)); // размер буфера строки в байтах
		_fullBuf = new unsigned long [_height_count];

		// Запрос памяти для аккумулятора абсолютных значений
		_sum_abs = new unsigned short [_height_count];
		cleanLineAccumulator();

		// блоки TBLENTRY плотно упакованы вслед за нулевым из DAQ_ASYNCREQDATA
		int ReqDataSize = sizeof(DAQ_ASYNCREQDATA); // используем толко один буфер!!!
		_ReqData = (DAQ_ASYNCREQDATA *)new char[ReqDataSize];
	
		// Подготовка к запросу буферов для АЦП.
		_ReqData->BufCnt = 1; // используем только один буфер
		TBLENTRY *pTbl = &_ReqData->Tbl[0]; // установим счётчик на нулевой TBLENTRY
		pTbl[0].Addr = NULL; // непрерывная область размера Tbl[i].Size запрашивается из системной памяти
		pTbl[0].Size = buf_size;

		// Request buffers of memory
		_RetStatus = DAQ_ioctl(_DAQ, DAQ_ioctlREAD_MEM_REQUEST, _ReqData); 
		if(_RetStatus != ERROR_SUCCESS)
			throw(DAQ_GetErrorMessage(_DAQ, _RetStatus));

		char *buffer = (char* )_ReqData->Tbl[0].Addr;
		memset( buffer, 0, buf_size );

		// Задаём частоту дискретизации АЦП, Гц.
		double Frq = _C/(2.*conf->getHeightStep());
		// Внести пожелания о частоте дискретизации АЦП.
		// Возвращает частоту дискретизации, реально установленную для АЦП.
		DAQ_ioctl(_DAQ, DAQ_ioctlSETRATE, &Frq);
		conf->setHeightStep(_C/(2.*Frq)); // реальный шаг по высоте, м
		_height_step = conf->getHeightStep(); // шаг по высоте, м

		// Настройка параметров асинхронного режима.
		_xferData.UseIrq = 1;	// 1 - использовать прерывания
		_xferData.Dir = 1;		// Направление обмена (1 - ввод, 2 - вывод данных)
		// Для непрерывной передачи данных по замкнутому циклу необходимо установить автоинициализацию обмена.
		_xferData.AutoInit = 0;	// 1 - автоинициализация обмена

		// Открываем LPT1 порт для записи через драйвер GiveIO.sys
		initLPT1();

		switch(conf->getMeasurement())
		{
		case IONOGRAM:
			// Определяем размер log-файла
			openIonogramFile(conf);
			break;
		case AMPLITUDES:
			openDataFile(conf);
			break;
		}
	}

	parusWork::~parusWork(void){
		// Free buffers of memory
		delete [] _fullBuf;
		delete [] _sum_abs;

		if(_DAQ)
			DAQ_ioctl(_DAQ, DAQ_ioctlREAD_MEM_RELEASE, NULL);
		_RetStatus = DAQ_close(_DAQ);
		if(_RetStatus != ERROR_SUCCESS)
			throw(DAQ_GetErrorMessage(_DAQ, _RetStatus));

		// Закрываем порт LPT
		CloseHandle(_LPTPort);

		// Закрываем выходной файл данных
		closeOutputFile();
	}

	// Подготовка запуска АЦП.
	M214x3M_DRVPARS parusWork::initADC(unsigned int count){
		M214x3M_DRVPARS DrvPars;
    
		DrvPars.Pars = M214x3M_Params; // Устанавливаем значения по умолчанию для субмодуля ADM214x1M			
		DrvPars.Carrier.Pars = AMB_Params; // Устанавливаем значения для базового модуля AMBPCI
		DrvPars.Carrier.Pars.AdcFifoSize = count; // размер буфера FIFO АЦП в 32-разрядных словах;
		// если ЦАП не установлен, то DacFifoSize должен быть равен 0,
		// иначе тесты на асинхронный вывод приведут к зависанию компьютера
		DrvPars.Carrier.Pars.DacFifoSize = 0; 

		// Включаем по одному входу в обоих каналах.
		DrvPars.Carrier.Pars.ChanMask = 257; // Если бит равен 1, то соответствующий вход включен, иначе - отключен.
		DrvPars.Pars.Gain[0].OnOff = 1;
		DrvPars.Pars.Gain[8].OnOff = 1;

		// Установка регистра режимов стартовой синхронизации АЦП
		DrvPars.Carrier.Pars.Start.Start = 1; // запуск по сигналу компаратора 0
		DrvPars.Carrier.Pars.Start.Src = 0;	// на компаратор 0 подается сигнал внешнего старта, на компаратор 1 подается сигнал с разъема X4
		DrvPars.Carrier.Pars.Start.Cmp0Inv = 1;	// 1 соответствует инверсии сигнала с выхода компаратора 0
		DrvPars.Carrier.Pars.Start.Cmp1Inv = 0;	// 1 соответствует инверсии сигнала с выхода компаратора 1
		DrvPars.Carrier.Pars.Start.Pretrig = 0;	// 1 соответствует режиму претриггера
		DrvPars.Carrier.Pars.Start.Trig.On = 1;	// 1 соответствует триггерному режиму запуска/остановки АЦП
		DrvPars.Carrier.Pars.Start.Trig.Stop = 0;// Режим остановки АЦП в триггерном режиме запуска: 0 - программная остановка
		DrvPars.Carrier.Pars.Start.Thr[0] = 1; // Пороговые значения в Вольтах для компараторов 0 и 1
		DrvPars.Carrier.Pars.Start.Thr[1] = 1;

		DrvPars.Carrier.HalPars = HAL_Params; // Устанавливаем значения по умолчанию для Слоя Аппаратных Абстракций (HAL)
											  // VendorID, DeviceID, InstanceDevice, LatencyTimer

		strcpy_s(DrvPars.Carrier.PldFileName, _PLDFileName.c_str()); // Используем старую версию  PLD файла

		return DrvPars;
	}

	// Инициирует процесс обмена данными между устройством и областью выделенной памяти, 
	// после чего возвращает управление программе, вызвавшей данную функцию.
	// При использовании прерываний перепрограммирование контроллера ПДП происходит по прерываниям. 
	// В противном случае используется периодический программный опрос статуса контроллера ПДП, 
	// в результате процесс непрерывной передачи данных существенно замедляется.
	void parusWork::ASYNC_TRANSFER(void){
		DAQ_ioctl(_DAQ, DAQ_ioctlASYNC_TRANSFER, &_xferData);
	}

	// Служит для проверки завершения асинхронного ввода данных в буфер. 
	// Функция возвращает ненулевое значение в случае завершения ввода, ноль – в противном случае
	int parusWork::READ_BUFISCOMPLETE(unsigned long msTimeout){
		return DAQ_ioctl(_DAQ, DAQ_ioctlREAD_BUFISCOMPLETE, &msTimeout);
	}

	// Позволяет узнать текущее состояние процесса асинхронного ввода данных. 
	// Функция возвращает номер текущего буфера (нумеруются с нуля), размер введенных 
	// в текущий буфер на момент запроса данных (в байтах), число произведенных автоинициализаций.
	void parusWork::READ_GETIOSTATE(void){
		DAQ_ioctl(_DAQ, DAQ_ioctlREAD_GETIOSTATE, &_curState);
	}

	// Позволяет прервать процесс асинхронного ввода данных. 
	// Функция возвращает номер текущего (на момент завершения процесса) 
	// буфера (нумеруются с нуля), размер введенных в текущий буфер на момент 
	// завершения процесса данных (в байтах), число произведенных автоинициализаций.
	void parusWork::READ_ABORTIO(void){
		DAQ_ioctl(_DAQ, DAQ_ioctlREAD_ABORTIO, &_curState);
	}

	// Служит для проверки завершения ввода данных, инициированного функцией начала 
	// обмена данными (DAQ_ioctlASYNC_TRANSFER). Функция возвращает ненулевое значение 
	// в случае завершения ввода, ноль – в противном случае.
	int parusWork::READ_ISCOMPLETE(unsigned long msTimeout){
		return DAQ_ioctl(_DAQ, DAQ_ioctlREAD_ISCOMPLETE, &msTimeout);
	}

	// Программируем синтезатор приемника
	void parusWork::adjustSounding(unsigned int curFrq){
	// curFrq -  частота, Гц
	// g - усиление
	// att - ослабление (вкл/выкл)
		char			ag, sb, j, jk, jk1, j1, j2, jk2;
		int				i;
		double			step = 2.5;
		unsigned int	fp = 63000, nf, r = 4;
		unsigned int	s_gr[9] = {0, 3555, 8675, 13795, 18915, 24035, 29155, 34280, 39395};
		int				Address=888;

		_curFrq = curFrq;

		for ( i = 8; i > 0; )
			if ( curFrq > s_gr[--i] ){
				sb = i;
				break;
			}
		nf =(unsigned int)( ((double)curFrq + (double)fp) / step );
	
		// Промежуточная частота и характеристики мощности.
		ag = _att + sb*2 + _g*16 + 128;
		_outp(Address, ag);

		// Запись шага по частоте в синтезатор
		for ( i = 2; i >= 0;  i-- ) { // Код шага частоты делителя синтезатора
			j = ( 0x1 & (r>>i) ) * 2 + 4;
			j1 = j + 1;
			jk = (j^0X0B)&0x0F;
			_outp( Address+2, jk );
			jk1 = (j1^0x0B)&0x0F;
			_outp(Address+2, jk1);
			_outp(Address+2, jk1);
			_outp(Address+2, jk);
		}

		// Запись кода частоты в синтезатор
		for ( i = 15; i >= 0; i-- )  { // Код частоты синтезатора
			j = ( 0x1 & (nf>>i) ) * 2 + 4;
			j1 = j2 = j + 1;
			if ( i == 0) 
				j2 -= 4;
			jk = (j^0X0B)&0X0F;
			_outp( Address+2, jk);
			jk1 = (j1^0X0B)&0X0F;
			_outp( Address+2, jk1);
			jk2 = (j2^0X0B)&0X0F;
			_outp( Address+2, jk2);
			_outp( Address+2, jk);
		}
	}

	// Программируем PIC на вывод синхроимпульсов сканирования ионограммы
	// Число строк, передаваемое в PIC-контроллер, можно задать с запасом на сбои.
	void parusWork::startGenerator(unsigned int nPulses){
	// nPulses -  количество импульсов генератора
	// fsound - частота следования строк, Гц
		double			fosc = 5e6;
		unsigned char	cdat[8] = {103, 159, 205, 218, 144, 1, 0, 0}; // 50 Гц, 398 строк (Вообще-то 400 строк. 2 запасные?)
		unsigned int	pimp_n, nimp_n, lnumb;
		unsigned long	NumberOfBytesWritten;
	
		lnumb = nPulses;

		// ==========================================================================================
		// Магические вычисления Геннадия Ивановича
		pimp_n = (unsigned int)(fosc/(4.*(double)_fsync));		// Циклов PIC на период	
		nimp_n = 0x1000d - pimp_n;								// PIC_TMR1       Fimp_min = 19 Hz
		cdat[0] = nimp_n%256;
		cdat[1] = nimp_n/256;
		cdat[2] = 0xCD;
		cdat[3] = 0xDA;
		cdat[4] = lnumb%256;
		cdat[5] = lnumb/256;
		cdat[6] = 0; // не используется
		cdat[7] = 0; // не используется

		// Завершение магических вычислений (Частота следования не менее 19 Гц.)
		// ==========================================================================================
 

		// ==========================================================================================
		// Запуск синхроимпульсов
		// ==========================================================================================
		// Передача параметров в кристалл PIC16F870
		HANDLE	_COMPort = initCOM2();
		BOOL fSuccess = WriteFile(
			_COMPort,		// описатель сом порта
			&cdat,		// Указатель на буфер, содержащий данные, которые будут записаны в файл. 
			8,// Число байтов, которые будут записаны в файл.
			&NumberOfBytesWritten,//  Указатель на переменную, которая получает число записанных байтов
			NULL // Указатель на структуру OVERLAPPED
		);
		if (!fSuccess){
			// Handle the error.
			throw std::runtime_error("Ошибка передачи параметров в кристалл PIC16F870.");
		}
	
		// Закрываем порт
		CloseHandle(_COMPort);
	}

	// Для передачи параметров в кристалл PIC16F870
	HANDLE parusWork::initCOM2(void){
		// Открываем порт для записи
		HANDLE _COMPort = CreateFile(TEXT("COM2"), 
								GENERIC_READ | GENERIC_WRITE, 
								0, 
								NULL, 
								OPEN_EXISTING, 
								0, 
								NULL);
		if (_COMPort == INVALID_HANDLE_VALUE) {
			throw std::runtime_error("Error! Невозможно открыть COM порт!");
		}
 
		// Build on the current configuration, and skip setting the size
		// of the input and output buffers with SetupComm.
		DCB dcb;
		BOOL fSuccess;

		// Проверка
		fSuccess = GetCommState(_COMPort, &dcb);
		if (!fSuccess){
			// Handle the error.
			throw std::runtime_error("Error! GetCommState failed with error!");
		}

		// Подготовка
		dcb.BaudRate = CBR_19200;     // set the baud rate
		dcb.ByteSize = 8;             // data size, xmit, and rcv
		dcb.Parity = NOPARITY;        // no parity bit
		dcb.StopBits = TWOSTOPBITS;   // two stop bit

		// Заполнение
		fSuccess = SetCommState(_COMPort, &dcb);
		if (!fSuccess){
			// Handle the error.
			throw std::runtime_error("Error! SetCommState failed with error!");
		}

		// Проверка
		fSuccess = GetCommState(_COMPort, &dcb);
		if (!fSuccess){
			// Handle the error.
			throw std::runtime_error("Error! GetCommState failed with error!");
		}

		return _COMPort;
	}

	// Для конфигурирования синтезатора.
	void parusWork::initLPT1(void){
		_LPTPort = CreateFile(TEXT("\\\\.\\giveio"),
							 GENERIC_READ,
							 0,
							 NULL,
							 OPEN_EXISTING,
							 FILE_ATTRIBUTE_NORMAL,
							 NULL);
		if (_LPTPort==INVALID_HANDLE_VALUE){
			throw std::runtime_error("Error! Can't open driver GIVEIO.SYS! Press any key to exit...");
		}
	}

	// Открываем файл ионограммы для записи и вносим в него заголовок.
	void parusWork::openIonogramFile(xmlconfig* conf)
	{
		// Заполнение заголовка файла.
		ionHeaderNew2 header = conf->getIonogramHeader();

		// Определение имя файла ионограммы.
		std::stringstream name;
		name << std::setfill('0');
		name << std::setw(4);
		name << header.time_sound.tm_year+1900 << std::setw(2);
		name << header.time_sound.tm_mon+1 << std::setw(2) 
			<< header.time_sound.tm_mday << std::setw(2) 
			<< header.time_sound.tm_hour << std::setw(2) 
			<< header.time_sound.tm_min << std::setw(2) << header.time_sound.tm_sec;
		name << ".ion";

		// Попытаемся открыть файл.
		_hFile = CreateFile(name.str().c_str(),		// name of the write
						   GENERIC_WRITE,			// open for writing
						   0,						// do not share
						   NULL,					// default security
						   CREATE_ALWAYS,			// create new file always
						   FILE_ATTRIBUTE_NORMAL,	// normal file
						   NULL);					// no attr. template
		if (_hFile == INVALID_HANDLE_VALUE)
			throw std::runtime_error("Ошибка открытия файла ионограмм на запись.");

		DWORD bytes;              // счётчик записи в файл
		BOOL Retval;
		Retval = WriteFile(_hFile, // писать в файл
				  &header,        // адрес буфера: что писать
				  sizeof(header), // сколько писать
				  &bytes,         // адрес DWORD'a: на выходе - сколько записано
				  0);             // A pointer to an OVERLAPPED structure.
		if (!Retval)
			throw std::runtime_error("Ошибка записи заголовка ионограммы в файл.");    
	}

	// Открываем файл данных для записи и вносим в него заголовок.
	void parusWork::openDataFile(xmlconfig* conf)
	{
		dataHeader header = conf->getAmplitudesHeader();

		// Определение имя файла данных.
		std::stringstream name;
		name << std::setfill('0');
		name << std::setw(4);
		name << header.time_sound.tm_year+1900 << std::setw(2);
		name << header.time_sound.tm_mon+1 << std::setw(2) 
			<< header.time_sound.tm_mday << std::setw(2) 
			<< header.time_sound.tm_hour << std::setw(2) 
			<< header.time_sound.tm_min << std::setw(2) << header.time_sound.tm_sec;
		name << ".frq";

		// Попытаемся открыть файл.
		_hFile = CreateFile(name.str().c_str(),		// name of the write
						   GENERIC_WRITE,			// open for writing
						   0,						// do not share
						   NULL,					// default security
						   CREATE_ALWAYS,			// create new file always
						   FILE_ATTRIBUTE_NORMAL,	// normal file
						   NULL);					// no attr. template
		if(_hFile == INVALID_HANDLE_VALUE)
			throw std::runtime_error("Ошибка открытия файла данных на запись.");

		DWORD bytes;              // счётчик записи в файл
		BOOL Retval;
		Retval = WriteFile(_hFile,	// писать в файл
				  &header,			// адрес буфера: что писать
				  sizeof(header),	// сколько писать
				  &bytes,			// адрес DWORD'a: на выходе - сколько записано
				  0);				// A pointer to an OVERLAPPED structure.
		for(unsigned i=0; i<header.count_modules; i++){ // последовательно пишем частоты зондирования
			unsigned f = conf->getAmplitudesFrq(i);
			Retval = WriteFile(_hFile,	// писать в файл
				  &f,					// адрес буфера: что писать
				  sizeof(f),			// сколько писать
				  &bytes,				// адрес DWORD'a: на выходе - сколько записано
				  0);					// A pointer to an OVERLAPPED structure.
		}
		if (!Retval)
			throw std::runtime_error("Ошибка записи заголовка файла данных.");    
	}

	void parusWork::cleanLineAccumulator(void)
	{
		for(size_t i = 0; i < _height_count; i++)
			_sum_abs[i] = 0;
	}

	void parusWork::accumulateLine(unsigned short curFrq)
	{
		// В буффере АЦП сохранены два сырых 16-битных чередующихся канала на одной частоте (count - количество 32-разрядных слов).
		memcpy(_fullBuf, getBuffer(), getBufferSize()); // копируем весь аппаратный буфер
		
		short re, im, abstmp;
		short max_abs_re = 0, max_abs_im = 0;
	    for(size_t i = 0; i < _height_count; i++)
		{
	        // Используем двухканальную интерпретацию через анонимную структуру
	        union {
	            unsigned long word;     // 4-байтное слово двухканального АЦП
	            adcTwoChannels twoCh;  // двухканальные (квадратурные) данные
	        };
	            
			// Разбиение на квадратуры. 
			// Значимы только старшие 14 бит. Младшие 2 бит - технологическая окраска.
	        word = _fullBuf[i];
	        /*re = static_cast<int>(twoCh.re.value) >> 2;
	        im = static_cast<int>(twoCh.im.value) >> 2;*/
			re = static_cast<short>(twoCh.re.value>>2);
	        im = static_cast<short>(twoCh.im.value>>2);

			max_abs_re = (abs(re) > abs(max_abs_re)) ? re : max_abs_re;
			max_abs_im = (abs(im) > abs(max_abs_im)) ? im : max_abs_im;
	
	        // Объединим квадратурную информацию в одну амплитуду.
			abstmp = static_cast<unsigned short>(floor(sqrt(re*re*1.0 + im*im*1.0)));
			_sum_abs[i] += abstmp;
		} 

		// Заполним журнал
		std::stringstream ss;
		ss << curFrq << ' ' << max_abs_re << ' ' << max_abs_im << ' ' <<
			((abs(max_abs_re) >= 8191 || abs(max_abs_im) >= 8191) ? "false" : "true");
		_log.push_back(ss.str());
	}

	void parusWork::averageLine(unsigned pulse_count)
	{
	    for(size_t i = 0; i < _height_count; i++)
			_sum_abs[i] /= pulse_count;  
	}

	void parusWork::saveDirtyLine(void)
	{
		// Writing data from buffer into file (unsigned long = unsigned int)
		BOOL	bErrorFlag = FALSE;
		DWORD	dwBytesWritten = 0;	
		bErrorFlag = WriteFile( 
			_hFile,				// open file handle
			_sum_abs,			// start of data to write
			_height_count * sizeof(unsigned short),// number of bytes to write
			&dwBytesWritten,	// number of bytes that were written
			NULL);				// no overlapped structure
		if (!bErrorFlag) 
			throw std::runtime_error("Не могу сохранить блок данных в файл.");
	}

	void parusWork::saveFullData(void)
	{
		// В буффере АЦП сохранены два сырых 16-битных чередующихся канала на одной частоте (count - количество 32-разрядных слов).
		memcpy(_fullBuf, getBuffer(), getBufferSize()); // копируем весь аппаратный буфер

		// Writing data from buffer into file (unsigned long = unsigned int)
		BOOL	bErrorFlag = FALSE;
		DWORD	dwBytesWritten = 0;	
		bErrorFlag = WriteFile( 
			_hFile,				// open file handle
			_fullBuf,			// start of data to write
			getBufferSize(),	// number of bytes to write
			&dwBytesWritten,	// number of bytes that were written
			NULL);				// no overlapped structure
		if (!bErrorFlag) 
			throw std::runtime_error("Не могу сохранить блок данных в файл.");
	}

	// 2017-09-13
	// Сохраняем строку данных с информацией об усилении приёмника и значении аттенюатора.
	// Это необходимо для отстройки по ограничениям.
	// Если есть ограничение сигнала, уменьшаем усиление на 3дБ для текущей частоты и перенастраиваем приемник.
	void parusWork::saveDataWithGain(void)
	{
		// В буффере АЦП сохранены два сырых 16-битных чередующихся канала на одной частоте (count - количество 32-разрядных слов).
		memcpy(_fullBuf, getBuffer(), getBufferSize()); // копируем весь аппаратный буфер

		// ------------------------------------------------------------------------------------------------------------------------
		// 1. Блок сохранения данных. Ограниченных или нет - без разницы.
		// ------------------------------------------------------------------------------------------------------------------------
		// Writing data from buffer into file (unsigned long = unsigned int)
		BOOL	bErrorFlag = FALSE;
		DWORD	dwBytesWritten = 0;	
		bErrorFlag = WriteFile( 
			_hFile,				// open file handle
			_fullBuf,			// start of data to write
			getBufferSize(),	//buf_size,// number of bytes to write
			&dwBytesWritten,	// number of bytes that were written
			NULL);				// no overlapped structure
		if (!bErrorFlag) 
			throw std::runtime_error("Не могу сохранить блок данных в файл.");

		// Сохранение усиления приемника (unsigned char)
		bErrorFlag = FALSE;
		dwBytesWritten = 0;
		unsigned char cur_gain = 46; // реальное усиление приёмника

		// _att - ослабление (аттенюатор) 1/0 = вкл/выкл
		if(_att) 
			cur_gain -= 12;
	
		//_g = conf.getGain()/6;	// ??? 6дБ = приращение в 4 раза по мощности
		//if(_g > 7) _g = 7;
		// min = +0 dB, max = +42 dB
		cur_gain += (unsigned char)_g * 6;

		bErrorFlag = WriteFile( 
			_hFile,				// open file handle
			&cur_gain,			// start of data to write - запись усиления приемника в дБ
			1,					// number of bytes to write
			&dwBytesWritten,	// number of bytes that were written
			NULL);				// no overlapped structure
		if (!bErrorFlag) 
			throw std::runtime_error("Не могу сохранить значение усиления приемника в файл.");
		// ------------------------------------------------------------------------------------------------------------------------
		// 1. Конец блока сохранения данных.
		// ------------------------------------------------------------------------------------------------------------------------

		// ------------------------------------------------------------------------------------------------------------------------
		// 2. Блок оценки возможности ограничения. Уменьшение усиления при выполнение условия. Учитывается только при последующих измерениях.
		// ------------------------------------------------------------------------------------------------------------------------
		short int *cur_ampl = (short int *)_fullBuf; // 2 байта на отсчет квадратуры, из которых 14 старших - значащие
		for ( unsigned long i = 0; i < 2 * getBufferSize() / sizeof(unsigned long); i++ )  { // перебор по измеренным значениям амплитуд
			if((cur_ampl[i] >> 2) > __AMPLITUDE_MAX__){ // реальный отсчёт амплитуды содержит 14 бит в старших разрядах
				_g -= 6;
				std::cout << "Уменьшение усиления приемника. Текущее усиление (относительно 46 дБ): " << _g << " дБ." << std::endl;
				break;
			}
		}
		// ------------------------------------------------------------------------------------------------------------------------
		// 2. Конец блока оценки возможности ограничения.
		// ------------------------------------------------------------------------------------------------------------------------
	}

	void parusWork::closeOutputFile(void)
	{
		if(_hFile)
			CloseHandle(_hFile);
	}

	// Усечение данных до char (сдвиг на 6 бит) и сохранение линии в файле.
	// Упаковка строки ионограммы по алгоритму ИЗМИРАН-ИПГ.
	// data - сырая строка ионограммы
	// conf - кофигурация зондирования
	// numModule - номер модуля зондирования
	// curFrq - частота зондирования, кГц
	void parusWork::saveLine(unsigned short curFrq)
	{
		// Неизменяемые данные по текущей частоте
		FrequencyData tmpFrequencyData;
		tmpFrequencyData.gain_control = static_cast<unsigned short>(_g * 6);			// !< Значение ослабления входного аттенюатора дБ.
		tmpFrequencyData.pulse_time = static_cast<unsigned short>(floor(1000./_fsync));	//!< Время зондирования на одной частоте, [мс].
		tmpFrequencyData.pulse_length = static_cast<unsigned char>(_pulse_duration);	//!< Длительность зондирующего импульса, [мкc].
		tmpFrequencyData.band = static_cast<unsigned char>(floor(1000000./_pulse_duration)); //!< Полоса сигнала, [кГц].
		tmpFrequencyData.type = 0;														//!< Вид модуляции (0 - гладкий импульс, 1 - ФКМ).

		// Выделим буфер под упакованную строку. Считаем, что упакованная строка не больше исходной.
		unsigned n = _height_count;
		unsigned char *tmpArray = new unsigned char [n];
		unsigned char *tmpLine = new unsigned char [n];
		unsigned char *tmpAmplitude = new unsigned char [n];
    
		unsigned j;
		unsigned char *dataLine = new unsigned char [n];

		// Усечение данных до размера 8 бит.
		for(unsigned i = 0; i < n; i++) 
			dataLine[i] = static_cast<unsigned char>(_sum_abs[i] >> 6);

		unsigned char thereshold;
		SignalResponse tmpSignalResponse;
		unsigned char countOutliers;
		unsigned short countFrq = 0; // количество байт в упакованном частотном массиве

		// Определим уровень наличия выбросов.
		thereshold = getThereshold(dataLine, n);            
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
				tmpSignalResponse.height_begin = static_cast<unsigned long>(floor(1.0 * j * _height_step));
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

		// Writing data from buffers into file (unsigned long = unsigned int)
		BOOL	bErrorFlag = FALSE;
		DWORD	dwBytesWritten = 0;	
		bErrorFlag = WriteFile( 
			_hFile,									// open file handle
			reinterpret_cast<char*>(tmpArray),		// start of data to write
			countLine + nFrequencyData,				// number of bytes to write
			&dwBytesWritten,						// number of bytes that were written
			NULL);									// no overlapped structure
		if (!bErrorFlag) 
			throw std::runtime_error("Не могу сохранить строку ионограммы в файл.");
	
		delete [] tmpLine;
		delete [] tmpAmplitude;
		delete [] tmpArray;
		delete [] dataLine;
	} 

	// Возвращает уровень более которого наблюдается выброс.
	unsigned char parusWork::getThereshold(unsigned char *arr, unsigned n)
	{
		unsigned char *sortData = new unsigned char [n]; // буферный массив для сортировки
		unsigned char Q1, Q3, dQ;
		unsigned short maxLim = 0;

		// 1. Упорядочим данные по возрастанию.
		memcpy(sortData, arr, n);
		qsort(sortData, n, sizeof(unsigned char), comp);
		// 2. Квартили (n - чётное, степень двойки!!!)
		Q1 = min(sortData[n/4],sortData[n/4-1]);
		Q3 = max(sortData[3*n/4],sortData[3*n/4-1]);
		// 3. Межквартильный диапазон
		dQ = Q3 - Q1;
		// 4. Верхняя граница выбросов.
		maxLim = Q3 + 3 * dQ;    

		delete [] sortData;

		return (maxLim>=255)? 255 : static_cast<unsigned char>(maxLim);
	}

	// сравнение целых
	int comp(const void *i, const void *j)
	{
		return *(unsigned char *)i - *(unsigned char *)j;
	}

} // namespace parus

	////	2.2.84	Получить коэффициент усиления (1002)
	////Позволяет получить текущий коэффициент усиления для произвольного входа.
	////Код управляющей функции:
	////M214x3M_ioctlGETGAIN = 1002
	////Тип аргумента:
	////указатель на структуру вида:
	////struct {
	////	float Value;		// Усиление
	////	int Input;		// Номер входа (0-15)
	////} M214x3M_GAINPARS;
	//	M214x3M_GAINPARS _GAIN;
	//	for(int kk = 0; kk<16; kk++){
	//		_GAIN.Input = kk;
	//		DAQ_ioctl(_DAQ, M214x3M_ioctlGETGAIN, &_GAIN);
	//	}

	////	2.2.86	Получить смещение нуля (1022)
	////Позволяет получить текущее смещение нуля для произвольного канала АЦП (половины входов).
	////Код управляющей функции:
	////M214x3M_ioctlGETINPUTOSV = 1022
	////Тип аргумента:
	////указатель на структуру вида:
	////struct {
	////	float Value;			// Смещение нуля
	////	int Chan;			// Номер канала (0 или 1)
	////} M214x3M_ZEROSHIFTPARS;
	//	M214x3M_ZEROSHIFTPARS _ZEROSHIFT;
	//	for(int kk = 0; kk<2; kk++){
	//		_ZEROSHIFT.Chan = kk;
	//		DAQ_ioctl(_DAQ, M214x3M_ioctlGETINPUTOSV, &_ZEROSHIFT);
	//	}

	////	2.2.88	Получить включенные входы и их усиление (1024)
	////Позволяет узнать номера опрашиваемых входов и их коэффициенты усиления.
	////Код управляющей функции:
	////M214x3M_ioctlGETINP = 1024,
	////Тип аргумента:
	////указатель на структуру вида:
	////struct{
	////  int	Counter;	// Число включенных входов
	////	struct {	
	////  float Value;		// Усиление
	////  int  Input;		// Номер входа (0-15)
	////} M214x3M_GAINPARS Gain[16]; 	// Номера и усиления включенных входов
	////} M214x3M_INPPARS;
	//	M214x3M_INPPARS _INPPARS;
	//	for(int kk = 0; kk<16; kk++){
	//		_INPPARS.Gain->Input = kk;
	//		DAQ_ioctl(_DAQ, M214x3M_ioctlGETINP, &_INPPARS);
	//	}