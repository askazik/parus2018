// ====================================================================================
// Класс работы с аппаратурой
// ====================================================================================
#include "ParusWork.h"

namespace parus {

	const std::string parusWork::_PLDFileName = "p14x3m31.hex";
	const std::string parusWork::_DriverName = "m14x3m.dll";
	const std::string parusWork::_DeviceName = "ADM214x3M on AMBPCI";
	const double parusWork::_C = 2.9979e8; // скорость света в вакууме

	parusWork::parusWork(void) :
		_hFile(NULL),
		_height_count(__COUNT_MAX__)
	{	
		// Открываем устройство, используя глобальные переменные, установленные заранее.	
		_DrvPars = initADC(_height_count);
		_DAQ = DAQ_open(_DriverName.c_str(), &_DrvPars); // NULL 
		if(!_DAQ)
			throw std::runtime_error("Не открывается модуль сбора данных.");
				
		// =================================================================================================
		// Выделение буферов для обработки сигнала.
		// =================================================================================================
		// Запрос памяти для двух сырых 16-битных чередующихся каналов (из АЦП получаем 32-разрядное слово на отсчёт).
		buf_size = static_cast<unsigned long>(_height_count * sizeof(unsigned long)); // размер буфера строки в байтах
		_fullBuf = new unsigned long [_height_count];

		// =================================================================================================
		// Выделение буфера для работы АЦП.
		// =================================================================================================
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
		// =================================================================================================

		// Настройка параметров асинхронного режима.
		_xferData.UseIrq = 1;	// 1 - использовать прерывания
		_xferData.Dir = 1;		// Направление обмена (1 - ввод, 2 - вывод данных)
		// Для непрерывной передачи данных по замкнутому циклу необходимо установить автоинициализацию обмена.
		_xferData.AutoInit = 0;	// 1 - автоинициализация обмена
		// Открываем LPT1 порт для записи через драйвер GiveIO.sys
		initLPT1();
	}

	void parusWork::setup(xml_unit* conf)
	{
		_g = conf->getGain()/6;	// ??? 6дБ = приращение в 4 раза по мощности
		if(_g > 7) _g = 7;

		_att = static_cast<char>(conf->getAttenuation());
		_fsync = conf->getPulseFrq();
		_pulse_duration = conf->getPulseDuration();
		_height_count = conf->getHeightCount(); // количество высот (реально измеренных)
		_height_min = 0; // начальная высота, м

		// Задаём частоту дискретизации АЦП, Гц.
		double Frq = _C/(2.*conf->getHeightStep());
		// Внести пожелания о частоте дискретизации АЦП.
		// Возвращает частоту дискретизации, реально установленную для АЦП.
		DAQ_ioctl(_DAQ, DAQ_ioctlSETRATE, &Frq);
		conf->setHeightStep(_C/(2.*Frq)); // реальный шаг по высоте, м
		_height_step = conf->getHeightStep(); // шаг по высоте, м

		switch(conf->getMeasurement())
		{
		case IONOGRAM:
			// Определяем размер log-файла
			openIonogramFile((xml_ionogram*)conf);
			break;
		case AMPLITUDES:
			openDataFile((xml_amplitudes*)conf);
			break;
		}
	}

	parusWork::~parusWork(void){
		// Free buffers of memory
		delete [] _fullBuf;

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
	int parusWork::GETGAIN(M214x3M_GAINPARS _GAIN){
		_RetStatus = DAQ_ioctl(_DAQ, M214x3M_ioctlGETGAIN, &_GAIN);
		if(_RetStatus != ERROR_SUCCESS)
			throw(DAQ_GetErrorMessage(_DAQ, _RetStatus));

		return _RetStatus;
	}

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
	int parusWork::GETINPUTOSV(M214x3M_ZEROSHIFTPARS _ZEROSHIFT)
	{
		_RetStatus = DAQ_ioctl(_DAQ, M214x3M_ioctlGETINPUTOSV, &_ZEROSHIFT);
		if(_RetStatus != ERROR_SUCCESS)
			throw(DAQ_GetErrorMessage(_DAQ, _RetStatus));

		return _RetStatus;
	}

	// Устанавливает смещение нуля (в диапазоне +/- 2.5 B) для произвольного канала АЦП (половины входов). 
	// Если Chan=0, то для входов 0-7. 
	// Если Chan=1, то для входов 8-15.
	int parusWork::SETINPUTOSV(M214x3M_ZEROSHIFTPARS _ZEROSHIFT)
	{					
		_RetStatus = DAQ_ioctl(_DAQ, M214x3M_ioctlSETINPUTOSV, &_ZEROSHIFT); // Установим смещение нуля
		if(_RetStatus != ERROR_SUCCESS)
			throw(DAQ_GetErrorMessage(_DAQ, _RetStatus));

		return _RetStatus;
	}
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
	int parusWork::GETINP(M214x3M_INPPARS _INPPARS)
	{
		_RetStatus = DAQ_ioctl(_DAQ, M214x3M_ioctlGETINP, &_INPPARS);
		if(_RetStatus != ERROR_SUCCESS)
			throw(DAQ_GetErrorMessage(_DAQ, _RetStatus));

		return _RetStatus;
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
	void parusWork::openIonogramFile(xml_ionogram* conf)
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
	void parusWork::openDataFile(xml_amplitudes* conf)
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
	void parusWork::saveLine(char* buf, const size_t byte_counts, const unsigned int curFrq)
	{
		// Writing data from buffers into file (unsigned long = unsigned int)
		BOOL	bErrorFlag = FALSE;
		DWORD	dwBytesWritten = 0;	
		bErrorFlag = WriteFile( 
			_hFile,						// open file handle
			buf,						// start of data to write
			byte_counts,				// number of bytes to write
			&dwBytesWritten,			// number of bytes that were written
			NULL);						// no overlapped structure
		if (!bErrorFlag)
		{
			std::stringstream ss;
			ss << "Не могу сохранить строку ионограммы в файл. Частота = " << curFrq << "кГц." << std::endl;
			throw std::runtime_error(ss.str());
		}
	}

	void parusWork::saveGain(void)
	{
		// Сохранение усиления приемника (unsigned char)
		BOOL	bErrorFlag = FALSE;
		DWORD	dwBytesWritten = 0;	
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
	}

	int parusWork::ionogram(xml_unit* conf)
	{
		xml_ionogram* ionogram = (xml_ionogram*)conf;
		DWORD msTimeout = 25;
		unsigned short curFrq; // текущая частота зондирования, кГц
		int counter; // число импульсов от генератора
		double zero_re = 0;
		double zero_im = 0;

		curFrq = ionogram->getModule(0)._map.at("fbeg");
		unsigned fstep = ionogram->getModule(0)._map.at("fstep");
		counter = ionogram->getFrequenciesCount() * ionogram->getPulseCount(); // требуемое число импульсов от генератора
		startGenerator(counter+1); // Запуск генератора импульсов.

		while(counter) // обрабатываем импульсы генератора
		{
			adjustSounding(curFrq);
			lineADC line(ionogram->getHeightCount());

			// !!!
			CBuffer buffer;
			buffer.setSavedSize(ionogram->getHeightCount());
			// !!!

			for (unsigned k = 0; k < ionogram->getPulseCount(); k++) // счётчик циклов суммирования на одной частоте
			{
				ASYNC_TRANSFER(); // запустим АЦП
				
				// Цикл проверки до появления результатов в буфере.
				// READ_BUFISCOMPLETE - сбоит на частоте 47 Гц
				while(READ_ISCOMPLETE(msTimeout) == NULL);

				// Остановим АЦП
				READ_ABORTIO();					

				try
				{
					line.accumulate(getBuffer());

					// !!!
					buffer += getBuffer();
					// !!!
				}
				catch(CADCOverflowException &e) // Отлавливаем здесь только ошибки ограничения амплитуды.
				{
					// 1. Запись в журнал
					std::stringstream ss;
					ss << curFrq << '\t' << e.getHeightNumber() * ionogram->getHeightStep()  << '\t' 
						<< std::boolalpha << e.getOverflowRe() <<  '\t' << e.getOverflowIm();
					_log.push_back(ss.str());
					// 2. Уменьшение усиления сигнала (выполняется для последующей настройки <adjustSounding> частоты зондирования)
					_g--; // уменьшаем усиление на 6 дБ (вдвое по амплитуде)
					adjustSounding(curFrq);
				}
				counter--; // приступаем к обработке следующего импульса
			}
			// усредним по количеству импульсов зондирования на одной частоте
			if(ionogram->getPulseCount() > 1)
			{
				line.average(ionogram->getPulseCount());

				// !!!
				buffer /= ionogram->getPulseCount();
				// !!!
			}
			// Сохранение линии в файле.
			switch(ionogram->getVersion()) // подготовка массива char
			{
			case 0: // ИПГ
				line.prepareIPG_IonogramBuffer(ionogram,curFrq);
				break;
			case 1: // без упаковки данных (as is) и информации об усилении
				line.prepareDirty_IonogramBuffer();
				break;
			case 2: // для калибровки
				// saveFullData();
				break;
			case 3: // CDF

				break;
			}
			saveLine(line.returnIonogramBuffer(), line.getSavedSize(), curFrq); // Сохранение линии в файл.
			zero_re += line.getShiftRe(); // прибавим среднее значение по всем пульсам
			zero_im += line.getShiftIm(); // прибавим среднее значение по всем пульсам

			curFrq += fstep; // следующая частота зондирования
		}

		// Среднее смещение нуля по всем частотам и импульсам на одной частоте.
		zero_re /= ionogram->getFrequenciesCount();
		zero_im /= ionogram->getFrequenciesCount();
		
		// Изменение смещения нуля для АЦП.
		std::vector<double> zero_shift;
		zero_shift.push_back(zero_re);
		zero_shift.push_back(zero_im);
		adjustZeroShift(zero_shift);
		
		// Запись в журнал информации о смещении нуля
		std::stringstream ss;
		ss << "Смещение нуля: " << zero_re << " (Re), "	<< zero_im << " (Im).";
		_log.push_back(ss.str());

		// Сохраним информацию о наступлении ограничения сигнала
		std::vector<std::string> log = getLog();
		std::ofstream output_file("parus.log");
		std::ostream_iterator<std::string> output_iterator(output_file, "\n");
		std::copy(log.begin(), log.end(), output_iterator);

		return _RetStatus;
	}

	/// <summary>
	/// Настройка смещения нуля для обоих каналов.
	/// </summary>
	/// <param name="zero_shift">Смещение нуля нулевого и первого каналов, занесённые в вектор std::vector<double>.</param>
	void parusWork::adjustZeroShift(std::vector<double> zero_shift)
	{
		M214x3M_ZEROSHIFTPARS _ZEROSHIFT;

		for(size_t kk = 0; kk < 2; kk++)
		{
			_ZEROSHIFT.Chan = kk;
			_ZEROSHIFT.Value = 0;
			GETINPUTOSV(_ZEROSHIFT); // Получим смещение нуля

			// Изменение при несовпадении
			if(_ZEROSHIFT.Value != zero_shift.at(kk))
			{
				// Для коэффициента усиления АЦП = 1, при шкале +/- 2.5 В, шаг квантования примерно 0.305 мВ (2.5 / 8191 Вольт).
				float value = static_cast<float>(zero_shift.at(kk) * 2.5 / 8191); // 13 бит по абсолютному значению

				_ZEROSHIFT.Value = -value; // смещаем в обратном направлении
				SETINPUTOSV(_ZEROSHIFT);
			}
		}
	}

	/// <summary>
	/// Измерение амплитуд.
	/// </summary>
	/// <param name="conf">Конфигурация измерения.</param>
	/// <param name="dt">Время измерения в секундах.</param>
	/// <returns></returns>
	int parusWork::amplitudes(xml_unit* conf, unsigned dt)
	{
		xml_amplitudes* amplitudes = (xml_amplitudes*)conf;
		DWORD msTimeout = 25;
		unsigned short curFrq, numFrq = 0; // номер текущей частоты зондирования
		int counter; // число импульсов от генератора
		std::vector<unsigned int> G(amplitudes->getModulesCount(), _g); // вектор усилений для частот зондирования

		counter = dt * amplitudes->getPulseFrq(); // требуемое число импульсов от генератора
		startGenerator(counter+1); // Запуск генератора импульсов.

		while(counter) // обрабатываем импульсы генератора
		{
			curFrq = amplitudes->getAmplitudesFrq(numFrq); // заданная частота зондирования
			_g = G.at(numFrq); // усиление для заданной частоты зондирования
			adjustSounding(curFrq);

			lineADC line(amplitudes->getHeightCount());

			ASYNC_TRANSFER(); // запустим АЦП
				
			// Цикл проверки до появления результатов в буфере.
			// READ_BUFISCOMPLETE - сбоит на частоте 47 Гц
			while(READ_ISCOMPLETE(msTimeout) == NULL);

			// Остановим АЦП
			READ_ABORTIO();					

			try
			{
				line.accumulate(getBuffer());
			}
			catch(CADCOverflowException &e) // Отлавливаем здесь только ошибки ограничения амплитуды.
			{
				// 1. Запись в журнал
				std::stringstream ss;
				ss << curFrq << '\t' << e.getHeightNumber() * amplitudes->getHeightStep()  << '\t' 
					<< std::boolalpha << e.getOverflowRe() <<  '\t' << e.getOverflowIm();
				_log.push_back(ss.str());
				// 2. Уменьшение усиления сигнала (выполняется для последующей настройки частоты зондирования)
				G.at(numFrq) = G.at(numFrq) - 1; // уменьшаем усиление на 6 дБ (вдвое по амплитуде) для следующего импульса на частоте
			}

			// Сохранение линии в файле.
			switch(amplitudes->getVersion())
			{
				case 0: // ИПГ
					//work->saveDirtyLine();
					break;
				case 1: // без упаковки данных (as is) и информации об усилении
					line.prepareDirty_AmplitudesBuffer();
					break;
				case 2: // упаковка данных - существенное уменьшение размера файла
					//work->saveTheresholdLine();
					break;
				case 3: // обработка возможного ограничения сигнала для каждой частоты зондирования
					//work->saveDataWithGain();
					break;
			}
			saveLine(line.returnAmplitudesBuffer(), line.getSavedSize(), curFrq); // Сохранение линии в файл.
			saveGain(); // сохранение в файл усиления приемника на текущей частоте

			counter--; // приступаем к обработке следующего импульса
			
			numFrq++; // смена частоты зондирования
			if(numFrq == amplitudes->getModulesCount()) 
				numFrq = 0;
		}

		return _RetStatus;
	}

	void mySetPriorityClass(void)
	{
		HANDLE procHandle = GetCurrentProcess();
		DWORD priorityClass = GetPriorityClass(procHandle);

		if (!SetPriorityClass(procHandle, HIGH_PRIORITY_CLASS))
			std::cerr << "SetPriorityClass" << std::endl;

		priorityClass = GetPriorityClass(procHandle);
		std::cerr << "Priority Class is set to : ";
		switch(priorityClass)
		{
		case HIGH_PRIORITY_CLASS:
			std::cerr << "HIGH_PRIORITY_CLASS\r\n";
			break;
		case IDLE_PRIORITY_CLASS:
			std::cerr << "IDLE_PRIORITY_CLASS\r\n";
			break;
		case NORMAL_PRIORITY_CLASS:
			std::cerr << "NORMAL_PRIORITY_CLASS\r\n";
			break;
		case REALTIME_PRIORITY_CLASS:
			std::cerr << "REALTIME_PRIORITY_CLASS\r\n";
			break;
		default:
			std::cerr <<"Unknown priority class\r\n";
		}
	}
} // namespace parus