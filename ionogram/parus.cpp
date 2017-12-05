// ====================================================================================
// ����� ������ � �����������
// ====================================================================================
#include "parus.h"

namespace parus {

	const std::string parusWork::_PLDFileName = "p14x3m31.hex";
	const std::string parusWork::_DriverName = "m14x3m.dll";
	const std::string parusWork::_DeviceName = "ADM214x3M on AMBPCI";
	const double parusWork::_C = 2.9979e8; // �������� ����� � �������

	parusWork::parusWork(void) :
		_hFile(NULL)
	{
		_height_count = __COUNT_MAX__;
		
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

		// ������ ������ ��� ������������ ���������� ��������
		_sum_abs = new unsigned short [_height_count];
		cleanLineAccumulator();

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
		delete [] _sum_abs;

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

	void parusWork::cleanLineAccumulator(void)
	{
		for(size_t i = 0; i < _height_count; i++)
			_sum_abs[i] = 0;
	}

	void parusWork::accumulateLine(unsigned short curFrq)
	{
		// � ������� ��� ��������� ��� ����� 16-������ ������������ ������ �� ����� ������� (count - ���������� 32-��������� ����).
		memcpy(_fullBuf, getBuffer(), getBufferSize()); // �������� ���� ���������� �����
		
		short re, im, abstmp;
		short max_abs_re = 0, max_abs_im = 0;
	    for(size_t i = 0; i < _height_count; i++)
		{
	        // ���������� ������������� ������������� ����� ��������� ���������
	        union {
	            unsigned long word;     // 4-������� ����� �������������� ���
	            adcTwoChannels twoCh;  // ������������� (������������) ������
	        };
	            
			// ��������� �� ����������. 
			// ������� ������ ������� 14 ���. ������� 2 ��� - ��������������� �������.
	        word = _fullBuf[i];
	        /*re = static_cast<int>(twoCh.re.value) >> 2;
	        im = static_cast<int>(twoCh.im.value) >> 2;*/
			re = static_cast<short>(twoCh.re.value>>2);
	        im = static_cast<short>(twoCh.im.value>>2);

			max_abs_re = (abs(re) > abs(max_abs_re)) ? re : max_abs_re;
			max_abs_im = (abs(im) > abs(max_abs_im)) ? im : max_abs_im;
	
	        // ��������� ������������ ���������� � ���� ���������.
			abstmp = static_cast<unsigned short>(floor(sqrt(re*re*1.0 + im*im*1.0)));
			_sum_abs[i] += abstmp;
		} 

		// �������� ������
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
			throw std::runtime_error("�� ���� ��������� ���� ������ � ����.");
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
	void parusWork::saveLine(unsigned short curFrq)
	{
		// ������������ ������ �� ������� �������
		FrequencyData tmpFrequencyData;
		tmpFrequencyData.gain_control = static_cast<unsigned short>(_g * 6);			// !< �������� ���������� �������� ����������� ��.
		tmpFrequencyData.pulse_time = static_cast<unsigned short>(floor(1000./_fsync));	//!< ����� ������������ �� ����� �������, [��].
		tmpFrequencyData.pulse_length = static_cast<unsigned char>(_pulse_duration);	//!< ������������ ������������ ��������, [��c].
		tmpFrequencyData.band = static_cast<unsigned char>(floor(1000000./_pulse_duration)); //!< ������ �������, [���].
		tmpFrequencyData.type = 0;														//!< ��� ��������� (0 - ������� �������, 1 - ���).

		// ������� ����� ��� ����������� ������. �������, ��� ����������� ������ �� ������ ��������.
		unsigned n = _height_count;
		unsigned char *tmpArray = new unsigned char [n];
		unsigned char *tmpLine = new unsigned char [n];
		unsigned char *tmpAmplitude = new unsigned char [n];
    
		unsigned j;
		unsigned char *dataLine = new unsigned char [n];

		// �������� ������ �� ������� 8 ���.
		for(unsigned i = 0; i < n; i++) 
			dataLine[i] = static_cast<unsigned char>(_sum_abs[i] >> 6);

		unsigned char thereshold;
		SignalResponse tmpSignalResponse;
		unsigned char countOutliers;
		unsigned short countFrq = 0; // ���������� ���� � ����������� ��������� �������

		// ��������� ������� ������� ��������.
		thereshold = getThereshold(dataLine, n);            
		tmpFrequencyData.frequency = curFrq;
		tmpFrequencyData.threshold_o = thereshold;
		tmpFrequencyData.threshold_x = 0;
		tmpFrequencyData.count_o = 0; // ����� ����������
		tmpFrequencyData.count_x = 0; // ������� � ��� ���� �- � �- ���������� ����������.

		unsigned short countLine = 0; // ���������� ���� � ����������� ������
		countOutliers = 0; // ������� ���������� �������� � ������� ������
		j = 0;
		while(j < n) // ������� �������
		{
			if(dataLine[j] > thereshold) // ������ ������ - ������������ ���.
			{
				tmpSignalResponse.height_begin = static_cast<unsigned long>(floor(1.0 * j * _height_step));
				tmpSignalResponse.count_samples = 1;
				tmpAmplitude[tmpSignalResponse.count_samples-1] = dataLine[j];
				j++; // ������� � ���������� ��������
				while(dataLine[j] > thereshold && j < n)
				{
					tmpSignalResponse.count_samples++;
					tmpAmplitude[tmpSignalResponse.count_samples-1] = dataLine[j];
					j++; // ������� � ���������� ��������
				}
				countOutliers++; // ��������� ���������� �������� � ������

				// �������� ����������� �������
				// ��������� �������.
				unsigned short nn = sizeof(SignalResponse);
				memcpy(tmpLine+countLine, &tmpSignalResponse, nn);
				countLine += nn;
				// ������ �������.
				nn = tmpSignalResponse.count_samples*sizeof(unsigned char);
				memcpy(tmpLine+countLine, tmpAmplitude, nn);
				countLine += nn;
			}
			else
				j++; // ���� ��������� ������
		}

		// ������ ��� ������� ������� �������� � �����.
		// ��������� ������ ������� ������, ���� ���� ������� �����������.
		tmpFrequencyData.count_o = countOutliers;
		// ��������� �������.
		unsigned nFrequencyData = sizeof(FrequencyData);
		memcpy(tmpArray, &tmpFrequencyData, nFrequencyData);
		// ��������� ����� �������������� ������� ��������
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
			throw std::runtime_error("�� ���� ��������� ������ ���������� � ����.");
	
		delete [] tmpLine;
		delete [] tmpAmplitude;
		delete [] tmpArray;
		delete [] dataLine;
	} 

	// ���������� ������� ����� �������� ����������� ������.
	unsigned char parusWork::getThereshold(unsigned char *arr, unsigned n)
	{
		unsigned char *sortData = new unsigned char [n]; // �������� ������ ��� ����������
		unsigned char Q1, Q3, dQ;
		unsigned short maxLim = 0;

		// 1. ���������� ������ �� �����������.
		memcpy(sortData, arr, n);
		qsort(sortData, n, sizeof(unsigned char), comp);
		// 2. �������� (n - ������, ������� ������!!!)
		Q1 = min(sortData[n/4],sortData[n/4-1]);
		Q3 = max(sortData[3*n/4],sortData[3*n/4-1]);
		// 3. �������������� ��������
		dQ = Q3 - Q1;
		// 4. ������� ������� ��������.
		maxLim = Q3 + 3 * dQ;    

		delete [] sortData;

		return (maxLim>=255)? 255 : static_cast<unsigned char>(maxLim);
	}

	// ��������� �����
	int comp(const void *i, const void *j)
	{
		return *(unsigned char *)i - *(unsigned char *)j;
	}

} // namespace parus

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