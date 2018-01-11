// ====================================================================================
// ����� ������ � �����������
// ====================================================================================
#include "ParusWork.h"

namespace parus {

	const std::string parusWork::_PLDFileName = "p14x3m31.hex";
	const std::string parusWork::_DriverName = "m14x3m.dll";
	const std::string parusWork::_DeviceName = "ADM214x3M on AMBPCI";
	const double parusWork::_C = 2.9979e8; // �������� ����� � �������

	parusWork::parusWork(void) :
		_hFile(NULL),
		_height_count(__COUNT_MAX__)
	{	
		// ��������� ����������, ��������� ���������� ����������, ������������� �������.	
		_DrvPars = initADC(_height_count);
		_DAQ = DAQ_open(_DriverName.c_str(), &_DrvPars); // NULL 
		if(!_DAQ)
			throw std::runtime_error("�� ����������� ������ ����� ������.");
				
		// =================================================================================================
		// ��������� ������� ��� ��������� �������.
		// =================================================================================================
		// ������ ������ ��� ���� ����� 16-������ ������������ ������� (�� ��� �������� 32-��������� ����� �� ������).
		buf_size = static_cast<unsigned long>(_height_count * sizeof(unsigned long)); // ������ ������ ������ � ������
		_fullBuf = new unsigned long [_height_count];

		// =================================================================================================
		// ��������� ������ ��� ������ ���.
		// =================================================================================================
		// ����� TBLENTRY ������ ��������� ����� �� ������� �� DAQ_ASYNCREQDATA
		int ReqDataSize = sizeof(DAQ_ASYNCREQDATA); // ���������� ����� ���� �����!!!
		_ReqData = (DAQ_ASYNCREQDATA *)new char[ReqDataSize];
	
		// ���������� � ������� ������� ��� ���.
		_ReqData->BufCnt = 1; // ���������� ������ ���� �����
		TBLENTRY *pTbl = &_ReqData->Tbl[0]; // ��������� ������� �� ������� TBLENTRY
		pTbl[0].Addr = NULL; // ����������� ������� ������� Tbl[i].Size ������������� �� ��������� ������
		pTbl[0].Size = buf_size;

		// Request buffers of memory
		_RetStatus = DAQ_ioctl(_DAQ, DAQ_ioctlREAD_MEM_REQUEST, _ReqData); 
		if(_RetStatus != ERROR_SUCCESS)
			throw(DAQ_GetErrorMessage(_DAQ, _RetStatus));

		char *buffer = (char* )_ReqData->Tbl[0].Addr;
		memset( buffer, 0, buf_size );
		// =================================================================================================

		// ��������� ���������� ������������ ������.
		_xferData.UseIrq = 1;	// 1 - ������������ ����������
		_xferData.Dir = 1;		// ����������� ������ (1 - ����, 2 - ����� ������)
		// ��� ����������� �������� ������ �� ���������� ����� ���������� ���������� ����������������� ������.
		_xferData.AutoInit = 0;	// 1 - ����������������� ������
		// ��������� LPT1 ���� ��� ������ ����� ������� GiveIO.sys
		initLPT1();
	}

	void parusWork::setup(xml_unit* conf)
	{
		_g = conf->getGain()/6;	// ??? 6�� = ���������� � 4 ���� �� ��������
		if(_g > 7) _g = 7;

		_att = static_cast<char>(conf->getAttenuation());
		_fsync = conf->getPulseFrq();
		_pulse_duration = conf->getPulseDuration();
		_height_count = conf->getHeightCount(); // ���������� ����� (������� ����������)
		_height_min = 0; // ��������� ������, �

		// ����� ������� ������������� ���, ��.
		double Frq = _C/(2.*conf->getHeightStep());
		// ������ ��������� � ������� ������������� ���.
		// ���������� ������� �������������, ������� ������������� ��� ���.
		DAQ_ioctl(_DAQ, DAQ_ioctlSETRATE, &Frq);
		conf->setHeightStep(_C/(2.*Frq)); // �������� ��� �� ������, �
		_height_step = conf->getHeightStep(); // ��� �� ������, �

		switch(conf->getMeasurement())
		{
		case IONOGRAM:
			// ���������� ������ log-�����
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

		// ��������� ���� LPT
		CloseHandle(_LPTPort);

		// ��������� �������� ���� ������
		closeOutputFile();
	}

	// ���������� ������� ���.
	M214x3M_DRVPARS parusWork::initADC(unsigned int count){
		M214x3M_DRVPARS DrvPars;
    
		DrvPars.Pars = M214x3M_Params; // ������������� �������� �� ��������� ��� ��������� ADM214x1M			
		DrvPars.Carrier.Pars = AMB_Params; // ������������� �������� ��� �������� ������ AMBPCI
		DrvPars.Carrier.Pars.AdcFifoSize = count; // ������ ������ FIFO ��� � 32-��������� ������;
		// ���� ��� �� ����������, �� DacFifoSize ������ ���� ����� 0,
		// ����� ����� �� ����������� ����� �������� � ��������� ����������
		DrvPars.Carrier.Pars.DacFifoSize = 0; 

		// �������� �� ������ ����� � ����� �������.
		DrvPars.Carrier.Pars.ChanMask = 257; // ���� ��� ����� 1, �� ��������������� ���� �������, ����� - ��������.
		DrvPars.Pars.Gain[0].OnOff = 1;
		DrvPars.Pars.Gain[8].OnOff = 1;

		// ��������� �������� ������� ��������� ������������� ���
		DrvPars.Carrier.Pars.Start.Start = 1; // ������ �� ������� ����������� 0
		DrvPars.Carrier.Pars.Start.Src = 0;	// �� ���������� 0 �������� ������ �������� ������, �� ���������� 1 �������� ������ � ������� X4
		DrvPars.Carrier.Pars.Start.Cmp0Inv = 1;	// 1 ������������� �������� ������� � ������ ����������� 0
		DrvPars.Carrier.Pars.Start.Cmp1Inv = 0;	// 1 ������������� �������� ������� � ������ ����������� 1
		DrvPars.Carrier.Pars.Start.Pretrig = 0;	// 1 ������������� ������ �����������
		DrvPars.Carrier.Pars.Start.Trig.On = 1;	// 1 ������������� ����������� ������ �������/��������� ���
		DrvPars.Carrier.Pars.Start.Trig.Stop = 0;// ����� ��������� ��� � ���������� ������ �������: 0 - ����������� ���������
		DrvPars.Carrier.Pars.Start.Thr[0] = 1; // ��������� �������� � ������� ��� ������������ 0 � 1
		DrvPars.Carrier.Pars.Start.Thr[1] = 1;

		DrvPars.Carrier.HalPars = HAL_Params; // ������������� �������� �� ��������� ��� ���� ���������� ���������� (HAL)
											  // VendorID, DeviceID, InstanceDevice, LatencyTimer

		strcpy_s(DrvPars.Carrier.PldFileName, _PLDFileName.c_str()); // ���������� ������ ������  PLD �����

		return DrvPars;
	}

	// ���������� ������� ������ ������� ����� ����������� � �������� ���������� ������, 
	// ����� ���� ���������� ���������� ���������, ��������� ������ �������.
	// ��� ������������� ���������� �������������������� ����������� ��� ���������� �� �����������. 
	// � ��������� ������ ������������ ������������� ����������� ����� ������� ����������� ���, 
	// � ���������� ������� ����������� �������� ������ ����������� �����������.
	void parusWork::ASYNC_TRANSFER(void){
		DAQ_ioctl(_DAQ, DAQ_ioctlASYNC_TRANSFER, &_xferData);
	}

	// ������ ��� �������� ���������� ������������ ����� ������ � �����. 
	// ������� ���������� ��������� �������� � ������ ���������� �����, ���� � � ��������� ������
	int parusWork::READ_BUFISCOMPLETE(unsigned long msTimeout){
		return DAQ_ioctl(_DAQ, DAQ_ioctlREAD_BUFISCOMPLETE, &msTimeout);
	}

	// ��������� ������ ������� ��������� �������� ������������ ����� ������. 
	// ������� ���������� ����� �������� ������ (���������� � ����), ������ ��������� 
	// � ������� ����� �� ������ ������� ������ (� ������), ����� ������������� �����������������.
	void parusWork::READ_GETIOSTATE(void){
		DAQ_ioctl(_DAQ, DAQ_ioctlREAD_GETIOSTATE, &_curState);
	}

	// ��������� �������� ������� ������������ ����� ������. 
	// ������� ���������� ����� �������� (�� ������ ���������� ��������) 
	// ������ (���������� � ����), ������ ��������� � ������� ����� �� ������ 
	// ���������� �������� ������ (� ������), ����� ������������� �����������������.
	void parusWork::READ_ABORTIO(void){
		DAQ_ioctl(_DAQ, DAQ_ioctlREAD_ABORTIO, &_curState);
	}

	// ������ ��� �������� ���������� ����� ������, ��������������� �������� ������ 
	// ������ ������� (DAQ_ioctlASYNC_TRANSFER). ������� ���������� ��������� �������� 
	// � ������ ���������� �����, ���� � � ��������� ������.
	int parusWork::READ_ISCOMPLETE(unsigned long msTimeout){
		return DAQ_ioctl(_DAQ, DAQ_ioctlREAD_ISCOMPLETE, &msTimeout);
	}

	////	2.2.84	�������� ����������� �������� (1002)
	////��������� �������� ������� ����������� �������� ��� ������������� �����.
	////��� ����������� �������:
	////M214x3M_ioctlGETGAIN = 1002
	////��� ���������:
	////��������� �� ��������� ����:
	////struct {
	////	float Value;		// ��������
	////	int Input;		// ����� ����� (0-15)
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

	////	2.2.86	�������� �������� ���� (1022)
	////��������� �������� ������� �������� ���� ��� ������������� ������ ��� (�������� ������).
	////��� ����������� �������:
	////M214x3M_ioctlGETINPUTOSV = 1022
	////��� ���������:
	////��������� �� ��������� ����:
	////struct {
	////	float Value;			// �������� ����
	////	int Chan;			// ����� ������ (0 ��� 1)
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

	// ������������� �������� ���� (� ��������� +/- 2.5 B) ��� ������������� ������ ��� (�������� ������). 
	// ���� Chan=0, �� ��� ������ 0-7. 
	// ���� Chan=1, �� ��� ������ 8-15.
	int parusWork::SETINPUTOSV(M214x3M_ZEROSHIFTPARS _ZEROSHIFT)
	{					
		_RetStatus = DAQ_ioctl(_DAQ, M214x3M_ioctlSETINPUTOSV, &_ZEROSHIFT); // ��������� �������� ����
		if(_RetStatus != ERROR_SUCCESS)
			throw(DAQ_GetErrorMessage(_DAQ, _RetStatus));

		return _RetStatus;
	}
	////	2.2.88	�������� ���������� ����� � �� �������� (1024)
	////��������� ������ ������ ������������ ������ � �� ������������ ��������.
	////��� ����������� �������:
	////M214x3M_ioctlGETINP = 1024,
	////��� ���������:
	////��������� �� ��������� ����:
	////struct{
	////  int	Counter;	// ����� ���������� ������
	////	struct {	
	////  float Value;		// ��������
	////  int  Input;		// ����� ����� (0-15)
	////} M214x3M_GAINPARS Gain[16]; 	// ������ � �������� ���������� ������
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

	// ������������� ���������� ���������
	void parusWork::adjustSounding(unsigned int curFrq){
	// curFrq -  �������, ��
	// g - ��������
	// att - ���������� (���/����)
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
	
		// ������������� ������� � �������������� ��������.
		ag = _att + sb*2 + _g*16 + 128;
		_outp(Address, ag);

		// ������ ���� �� ������� � ����������
		for ( i = 2; i >= 0;  i-- ) { // ��� ���� ������� �������� �����������
			j = ( 0x1 & (r>>i) ) * 2 + 4;
			j1 = j + 1;
			jk = (j^0X0B)&0x0F;
			_outp( Address+2, jk );
			jk1 = (j1^0x0B)&0x0F;
			_outp(Address+2, jk1);
			_outp(Address+2, jk1);
			_outp(Address+2, jk);
		}

		// ������ ���� ������� � ����������
		for ( i = 15; i >= 0; i-- )  { // ��� ������� �����������
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

	// ������������� PIC �� ����� ��������������� ������������ ����������
	// ����� �����, ������������ � PIC-����������, ����� ������ � ������� �� ����.
	void parusWork::startGenerator(unsigned int nPulses){
	// nPulses -  ���������� ��������� ����������
	// fsound - ������� ���������� �����, ��
		double			fosc = 5e6;
		unsigned char	cdat[8] = {103, 159, 205, 218, 144, 1, 0, 0}; // 50 ��, 398 ����� (������-�� 400 �����. 2 ��������?)
		unsigned int	pimp_n, nimp_n, lnumb;
		unsigned long	NumberOfBytesWritten;
	
		lnumb = nPulses;

		// ==========================================================================================
		// ���������� ���������� �������� ���������
		pimp_n = (unsigned int)(fosc/(4.*(double)_fsync));		// ������ PIC �� ������	
		nimp_n = 0x1000d - pimp_n;								// PIC_TMR1       Fimp_min = 19 Hz
		cdat[0] = nimp_n%256;
		cdat[1] = nimp_n/256;
		cdat[2] = 0xCD;
		cdat[3] = 0xDA;
		cdat[4] = lnumb%256;
		cdat[5] = lnumb/256;
		cdat[6] = 0; // �� ������������
		cdat[7] = 0; // �� ������������

		// ���������� ���������� ���������� (������� ���������� �� ����� 19 ��.)
		// ==========================================================================================
 

		// ==========================================================================================
		// ������ ���������������
		// ==========================================================================================
		// �������� ���������� � �������� PIC16F870
		HANDLE	_COMPort = initCOM2();
		BOOL fSuccess = WriteFile(
			_COMPort,		// ��������� ��� �����
			&cdat,		// ��������� �� �����, ���������� ������, ������� ����� �������� � ����. 
			8,// ����� ������, ������� ����� �������� � ����.
			&NumberOfBytesWritten,//  ��������� �� ����������, ������� �������� ����� ���������� ������
			NULL // ��������� �� ��������� OVERLAPPED
		);
		if (!fSuccess){
			// Handle the error.
			throw std::runtime_error("������ �������� ���������� � �������� PIC16F870.");
		}
	
		// ��������� ����
		CloseHandle(_COMPort);
	}

	// ��� �������� ���������� � �������� PIC16F870
	HANDLE parusWork::initCOM2(void){
		// ��������� ���� ��� ������
		HANDLE _COMPort = CreateFile(TEXT("COM2"), 
								GENERIC_READ | GENERIC_WRITE, 
								0, 
								NULL, 
								OPEN_EXISTING, 
								0, 
								NULL);
		if (_COMPort == INVALID_HANDLE_VALUE) {
			throw std::runtime_error("Error! ���������� ������� COM ����!");
		}
 
		// Build on the current configuration, and skip setting the size
		// of the input and output buffers with SetupComm.
		DCB dcb;
		BOOL fSuccess;

		// ��������
		fSuccess = GetCommState(_COMPort, &dcb);
		if (!fSuccess){
			// Handle the error.
			throw std::runtime_error("Error! GetCommState failed with error!");
		}

		// ����������
		dcb.BaudRate = CBR_19200;     // set the baud rate
		dcb.ByteSize = 8;             // data size, xmit, and rcv
		dcb.Parity = NOPARITY;        // no parity bit
		dcb.StopBits = TWOSTOPBITS;   // two stop bit

		// ����������
		fSuccess = SetCommState(_COMPort, &dcb);
		if (!fSuccess){
			// Handle the error.
			throw std::runtime_error("Error! SetCommState failed with error!");
		}

		// ��������
		fSuccess = GetCommState(_COMPort, &dcb);
		if (!fSuccess){
			// Handle the error.
			throw std::runtime_error("Error! GetCommState failed with error!");
		}

		return _COMPort;
	}

	// ��� ���������������� �����������.
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

	// ��������� ���� ���������� ��� ������ � ������ � ���� ���������.
	void parusWork::openIonogramFile(xml_ionogram* conf)
	{
		// ���������� ��������� �����.
		ionHeaderNew2 header = conf->getIonogramHeader();

		// ����������� ��� ����� ����������.
		std::stringstream name;
		name << std::setfill('0');
		name << std::setw(4);
		name << header.time_sound.tm_year+1900 << std::setw(2);
		name << header.time_sound.tm_mon+1 << std::setw(2) 
			<< header.time_sound.tm_mday << std::setw(2) 
			<< header.time_sound.tm_hour << std::setw(2) 
			<< header.time_sound.tm_min << std::setw(2) << header.time_sound.tm_sec;
		name << ".ion";

		// ���������� ������� ����.
		_hFile = CreateFile(name.str().c_str(),		// name of the write
						   GENERIC_WRITE,			// open for writing
						   0,						// do not share
						   NULL,					// default security
						   CREATE_ALWAYS,			// create new file always
						   FILE_ATTRIBUTE_NORMAL,	// normal file
						   NULL);					// no attr. template
		if (_hFile == INVALID_HANDLE_VALUE)
			throw std::runtime_error("������ �������� ����� ��������� �� ������.");

		DWORD bytes;              // ������� ������ � ����
		BOOL Retval;
		Retval = WriteFile(_hFile, // ������ � ����
				  &header,        // ����� ������: ��� ������
				  sizeof(header), // ������� ������
				  &bytes,         // ����� DWORD'a: �� ������ - ������� ��������
				  0);             // A pointer to an OVERLAPPED structure.
		if (!Retval)
			throw std::runtime_error("������ ������ ��������� ���������� � ����.");    
	}

	// ��������� ���� ������ ��� ������ � ������ � ���� ���������.
	void parusWork::openDataFile(xml_amplitudes* conf)
	{
		dataHeader header = conf->getAmplitudesHeader();

		// ����������� ��� ����� ������.
		std::stringstream name;
		name << std::setfill('0');
		name << std::setw(4);
		name << header.time_sound.tm_year+1900 << std::setw(2);
		name << header.time_sound.tm_mon+1 << std::setw(2) 
			<< header.time_sound.tm_mday << std::setw(2) 
			<< header.time_sound.tm_hour << std::setw(2) 
			<< header.time_sound.tm_min << std::setw(2) << header.time_sound.tm_sec;
		name << ".frq";

		// ���������� ������� ����.
		_hFile = CreateFile(name.str().c_str(),		// name of the write
						   GENERIC_WRITE,			// open for writing
						   0,						// do not share
						   NULL,					// default security
						   CREATE_ALWAYS,			// create new file always
						   FILE_ATTRIBUTE_NORMAL,	// normal file
						   NULL);					// no attr. template
		if(_hFile == INVALID_HANDLE_VALUE)
			throw std::runtime_error("������ �������� ����� ������ �� ������.");

		DWORD bytes;              // ������� ������ � ����
		BOOL Retval;
		Retval = WriteFile(_hFile,	// ������ � ����
				  &header,			// ����� ������: ��� ������
				  sizeof(header),	// ������� ������
				  &bytes,			// ����� DWORD'a: �� ������ - ������� ��������
				  0);				// A pointer to an OVERLAPPED structure.
		for(unsigned i=0; i<header.count_modules; i++){ // ��������������� ����� ������� ������������
			unsigned f = conf->getAmplitudesFrq(i);
			Retval = WriteFile(_hFile,	// ������ � ����
				  &f,					// ����� ������: ��� ������
				  sizeof(f),			// ������� ������
				  &bytes,				// ����� DWORD'a: �� ������ - ������� ��������
				  0);					// A pointer to an OVERLAPPED structure.
		}
		if (!Retval)
			throw std::runtime_error("������ ������ ��������� ����� ������.");    
	}

	void parusWork::saveFullData(void)
	{
		// � ������� ��� ��������� ��� ����� 16-������ ������������ ������ �� ����� ������� (count - ���������� 32-��������� ����).
		memcpy(_fullBuf, getBuffer(), getBufferSize()); // �������� ���� ���������� �����

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
			throw std::runtime_error("�� ���� ��������� ���� ������ � ����.");
	}

	// 2017-09-13
	// ��������� ������ ������ � ����������� �� �������� �������� � �������� �����������.
	// ��� ���������� ��� ��������� �� ������������.
	// ���� ���� ����������� �������, ��������� �������� �� 3�� ��� ������� ������� � ��������������� ��������.
	void parusWork::saveDataWithGain(void)
	{
		// � ������� ��� ��������� ��� ����� 16-������ ������������ ������ �� ����� ������� (count - ���������� 32-��������� ����).
		memcpy(_fullBuf, getBuffer(), getBufferSize()); // �������� ���� ���������� �����

		// ------------------------------------------------------------------------------------------------------------------------
		// 1. ���� ���������� ������. ������������ ��� ��� - ��� �������.
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
			throw std::runtime_error("�� ���� ��������� ���� ������ � ����.");

		// ���������� �������� ��������� (unsigned char)
		bErrorFlag = FALSE;
		dwBytesWritten = 0;
		unsigned char cur_gain = 46; // �������� �������� ��������

		// _att - ���������� (����������) 1/0 = ���/����
		if(_att) 
			cur_gain -= 12;
	
		//_g = conf.getGain()/6;	// ??? 6�� = ���������� � 4 ���� �� ��������
		//if(_g > 7) _g = 7;
		// min = +0 dB, max = +42 dB
		cur_gain += (unsigned char)_g * 6;

		bErrorFlag = WriteFile( 
			_hFile,				// open file handle
			&cur_gain,			// start of data to write - ������ �������� ��������� � ��
			1,					// number of bytes to write
			&dwBytesWritten,	// number of bytes that were written
			NULL);				// no overlapped structure
		if (!bErrorFlag) 
			throw std::runtime_error("�� ���� ��������� �������� �������� ��������� � ����.");
		// ------------------------------------------------------------------------------------------------------------------------
		// 1. ����� ����� ���������� ������.
		// ------------------------------------------------------------------------------------------------------------------------

		// ------------------------------------------------------------------------------------------------------------------------
		// 2. ���� ������ ����������� �����������. ���������� �������� ��� ���������� �������. ����������� ������ ��� ����������� ����������.
		// ------------------------------------------------------------------------------------------------------------------------
		short int *cur_ampl = (short int *)_fullBuf; // 2 ����� �� ������ ����������, �� ������� 14 ������� - ��������
		for ( unsigned long i = 0; i < 2 * getBufferSize() / sizeof(unsigned long); i++ )  { // ������� �� ���������� ��������� ��������
			if((cur_ampl[i] >> 2) > __AMPLITUDE_MAX__){ // �������� ������ ��������� �������� 14 ��� � ������� ��������
				_g -= 6;
				std::cout << "���������� �������� ���������. ������� �������� (������������ 46 ��): " << _g << " ��." << std::endl;
				break;
			}
		}
		// ------------------------------------------------------------------------------------------------------------------------
		// 2. ����� ����� ������ ����������� �����������.
		// ------------------------------------------------------------------------------------------------------------------------
	}

	void parusWork::closeOutputFile(void)
	{
		if(_hFile)
			CloseHandle(_hFile);
	}

	// �������� ������ �� char (����� �� 6 ���) � ���������� ����� � �����.
	// �������� ������ ���������� �� ��������� �������-���.
	// data - ����� ������ ����������
	// conf - ����������� ������������
	// numModule - ����� ������ ������������
	// curFrq - ������� ������������, ���
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
			ss << "�� ���� ��������� ������ ���������� � ����. ������� = " << curFrq << "���." << std::endl;
			throw std::runtime_error(ss.str());
		}
	}

	void parusWork::saveGain(void)
	{
		// ���������� �������� ��������� (unsigned char)
		BOOL	bErrorFlag = FALSE;
		DWORD	dwBytesWritten = 0;	
		unsigned char cur_gain = 46; // �������� �������� ��������

		// _att - ���������� (����������) 1/0 = ���/����
		if(_att) 
			cur_gain -= 12;
	
		//_g = conf.getGain()/6;	// ??? 6�� = ���������� � 4 ���� �� ��������
		//if(_g > 7) _g = 7;
		// min = +0 dB, max = +42 dB
		cur_gain += (unsigned char)_g * 6;

		bErrorFlag = WriteFile( 
			_hFile,				// open file handle
			&cur_gain,			// start of data to write - ������ �������� ��������� � ��
			1,					// number of bytes to write
			&dwBytesWritten,	// number of bytes that were written
			NULL);				// no overlapped structure
		if (!bErrorFlag) 
			throw std::runtime_error("�� ���� ��������� �������� �������� ��������� � ����.");
	}

	int parusWork::ionogram(xml_unit* conf)
	{
		xml_ionogram* ionogram = (xml_ionogram*)conf;
		DWORD msTimeout = 25;
		unsigned short curFrq; // ������� ������� ������������, ���
		int counter; // ����� ��������� �� ����������
		double zero_re = 0;
		double zero_im = 0;

		curFrq = ionogram->getModule(0)._map.at("fbeg");
		unsigned fstep = ionogram->getModule(0)._map.at("fstep");
		counter = ionogram->getFrequenciesCount() * ionogram->getPulseCount(); // ��������� ����� ��������� �� ����������
		startGenerator(counter+1); // ������ ���������� ���������.

		while(counter) // ������������ �������� ����������
		{
			adjustSounding(curFrq);
			lineADC line(ionogram->getHeightCount());

			// !!!
			CBuffer buffer;
			buffer.setSavedSize(ionogram->getHeightCount());
			// !!!

			for (unsigned k = 0; k < ionogram->getPulseCount(); k++) // ������� ������ ������������ �� ����� �������
			{
				ASYNC_TRANSFER(); // �������� ���
				
				// ���� �������� �� ��������� ����������� � ������.
				// READ_BUFISCOMPLETE - ����� �� ������� 47 ��
				while(READ_ISCOMPLETE(msTimeout) == NULL);

				// ��������� ���
				READ_ABORTIO();					

				try
				{
					line.accumulate(getBuffer());

					// !!!
					buffer += getBuffer();
					// !!!
				}
				catch(CADCOverflowException &e) // ����������� ����� ������ ������ ����������� ���������.
				{
					// 1. ������ � ������
					std::stringstream ss;
					ss << curFrq << '\t' << e.getHeightNumber() * ionogram->getHeightStep()  << '\t' 
						<< std::boolalpha << e.getOverflowRe() <<  '\t' << e.getOverflowIm();
					_log.push_back(ss.str());
					// 2. ���������� �������� ������� (����������� ��� ����������� ��������� <adjustSounding> ������� ������������)
					_g--; // ��������� �������� �� 6 �� (����� �� ���������)
					adjustSounding(curFrq);
				}
				counter--; // ���������� � ��������� ���������� ��������
			}
			// �������� �� ���������� ��������� ������������ �� ����� �������
			if(ionogram->getPulseCount() > 1)
			{
				line.average(ionogram->getPulseCount());

				// !!!
				buffer /= ionogram->getPulseCount();
				// !!!
			}
			// ���������� ����� � �����.
			switch(ionogram->getVersion()) // ���������� ������� char
			{
			case 0: // ���
				line.prepareIPG_IonogramBuffer(ionogram,curFrq);
				break;
			case 1: // ��� �������� ������ (as is) � ���������� �� ��������
				line.prepareDirty_IonogramBuffer();
				break;
			case 2: // ��� ����������
				// saveFullData();
				break;
			case 3: // CDF

				break;
			}
			saveLine(line.returnIonogramBuffer(), line.getSavedSize(), curFrq); // ���������� ����� � ����.
			zero_re += line.getShiftRe(); // �������� ������� �������� �� ���� �������
			zero_im += line.getShiftIm(); // �������� ������� �������� �� ���� �������

			curFrq += fstep; // ��������� ������� ������������
		}

		// ������� �������� ���� �� ���� �������� � ��������� �� ����� �������.
		zero_re /= ionogram->getFrequenciesCount();
		zero_im /= ionogram->getFrequenciesCount();
		
		// ��������� �������� ���� ��� ���.
		std::vector<double> zero_shift;
		zero_shift.push_back(zero_re);
		zero_shift.push_back(zero_im);
		adjustZeroShift(zero_shift);
		
		// ������ � ������ ���������� � �������� ����
		std::stringstream ss;
		ss << "�������� ����: " << zero_re << " (Re), "	<< zero_im << " (Im).";
		_log.push_back(ss.str());

		// �������� ���������� � ����������� ����������� �������
		std::vector<std::string> log = getLog();
		std::ofstream output_file("parus.log");
		std::ostream_iterator<std::string> output_iterator(output_file, "\n");
		std::copy(log.begin(), log.end(), output_iterator);

		return _RetStatus;
	}

	/// <summary>
	/// ��������� �������� ���� ��� ����� �������.
	/// </summary>
	/// <param name="zero_shift">�������� ���� �������� � ������� �������, ��������� � ������ std::vector<double>.</param>
	void parusWork::adjustZeroShift(std::vector<double> zero_shift)
	{
		M214x3M_ZEROSHIFTPARS _ZEROSHIFT;

		for(size_t kk = 0; kk < 2; kk++)
		{
			_ZEROSHIFT.Chan = kk;
			_ZEROSHIFT.Value = 0;
			GETINPUTOSV(_ZEROSHIFT); // ������� �������� ����

			// ��������� ��� ������������
			if(_ZEROSHIFT.Value != zero_shift.at(kk))
			{
				// ��� ������������ �������� ��� = 1, ��� ����� +/- 2.5 �, ��� ����������� �������� 0.305 �� (2.5 / 8191 �����).
				float value = static_cast<float>(zero_shift.at(kk) * 2.5 / 8191); // 13 ��� �� ����������� ��������

				_ZEROSHIFT.Value = -value; // ������� � �������� �����������
				SETINPUTOSV(_ZEROSHIFT);
			}
		}
	}

	/// <summary>
	/// ��������� ��������.
	/// </summary>
	/// <param name="conf">������������ ���������.</param>
	/// <param name="dt">����� ��������� � ��������.</param>
	/// <returns></returns>
	int parusWork::amplitudes(xml_unit* conf, unsigned dt)
	{
		xml_amplitudes* amplitudes = (xml_amplitudes*)conf;
		DWORD msTimeout = 25;
		unsigned short curFrq, numFrq = 0; // ����� ������� ������� ������������
		int counter; // ����� ��������� �� ����������
		std::vector<unsigned int> G(amplitudes->getModulesCount(), _g); // ������ �������� ��� ������ ������������

		counter = dt * amplitudes->getPulseFrq(); // ��������� ����� ��������� �� ����������
		startGenerator(counter+1); // ������ ���������� ���������.

		while(counter) // ������������ �������� ����������
		{
			curFrq = amplitudes->getAmplitudesFrq(numFrq); // �������� ������� ������������
			_g = G.at(numFrq); // �������� ��� �������� ������� ������������
			adjustSounding(curFrq);

			lineADC line(amplitudes->getHeightCount());

			ASYNC_TRANSFER(); // �������� ���
				
			// ���� �������� �� ��������� ����������� � ������.
			// READ_BUFISCOMPLETE - ����� �� ������� 47 ��
			while(READ_ISCOMPLETE(msTimeout) == NULL);

			// ��������� ���
			READ_ABORTIO();					

			try
			{
				line.accumulate(getBuffer());
			}
			catch(CADCOverflowException &e) // ����������� ����� ������ ������ ����������� ���������.
			{
				// 1. ������ � ������
				std::stringstream ss;
				ss << curFrq << '\t' << e.getHeightNumber() * amplitudes->getHeightStep()  << '\t' 
					<< std::boolalpha << e.getOverflowRe() <<  '\t' << e.getOverflowIm();
				_log.push_back(ss.str());
				// 2. ���������� �������� ������� (����������� ��� ����������� ��������� ������� ������������)
				G.at(numFrq) = G.at(numFrq) - 1; // ��������� �������� �� 6 �� (����� �� ���������) ��� ���������� �������� �� �������
			}

			// ���������� ����� � �����.
			switch(amplitudes->getVersion())
			{
				case 0: // ���
					//work->saveDirtyLine();
					break;
				case 1: // ��� �������� ������ (as is) � ���������� �� ��������
					line.prepareDirty_AmplitudesBuffer();
					break;
				case 2: // �������� ������ - ������������ ���������� ������� �����
					//work->saveTheresholdLine();
					break;
				case 3: // ��������� ���������� ����������� ������� ��� ������ ������� ������������
					//work->saveDataWithGain();
					break;
			}
			saveLine(line.returnAmplitudesBuffer(), line.getSavedSize(), curFrq); // ���������� ����� � ����.
			saveGain(); // ���������� � ���� �������� ��������� �� ������� �������

			counter--; // ���������� � ��������� ���������� ��������
			
			numFrq++; // ����� ������� ������������
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