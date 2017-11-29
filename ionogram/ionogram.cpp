// ===========================================================================
// ������ ���������� � ����.
// ===========================================================================
#include "ionogram.h"

using namespace parus;

int main(void)
{	
	setlocale(LC_ALL,"Russian"); // ��������� ������ �� ����� ��������� ��-������
	SetPriorityClass();

    // ===========================================================================================
    // 1. ������ ���� ������������ ��� ��������� ����������.
    // ===========================================================================================
	xmlunit unit(IONOGRAM);

	xmlconfig conf;
	ionHeaderNew2 _header = conf.getIonogramHeader();

	std::cout << "���������� ���������������� ����: <" << conf.getFileName() << ">." << std::endl;

	std::cout << "��������� ������������: " << std::endl;
	std::cout << "�������: " << _header.freq_min << " ��� - " << _header.freq_max 
				<< " ���, �������� = " << conf.getGain() 
				<< " ��, ���������� = " << conf.getAttenuation() << " ����(0)/���(1)." << std::endl;

    // ===========================================================================================
    // 1. ������ ���� ������������ ��� ����������� ���������.
    // ===========================================================================================
	//xmlconfig conf("config.xml", AMPLITUDES);

	//std::cout << "���������� ���������������� ����: <" << conf.getFileName() << ">." << std::endl;

	//std::cout << "��������� ������������: " << std::endl;
	//std::cout << "�������� = " << conf.getGain() 
	//			<< " ��, ���������� = " << conf.getAttenuation() << " ����(0)/���(1)." << std::endl;
	//for(size_t i = 0; i < conf.getModulesCount(); i++)
	//	std::cout << conf.getAmplitudesFrq(i) << " ���" << std::endl;

    // ===========================================================================================
    // 2. ���������������� ������.
    // ===========================================================================================
	int RetStatus = 0;
	try	
	{
		// ���������� ���������� � ������������.
		// �������� �������� ����� ������ � ������ ���������.
		ionogramSettings ion = conf.getIonogramSettings();
		parusWork *work = new parusWork(&conf);
		// �������� ��� ���������� ������������� ADC
		Sleep(500); // ����������

		DWORD msTimeout = 25;
		unsigned short curFrq = ion.fbeg; // ������� ������� ������������, ���
		int counter = ion.count * conf.getPulseCount(); // ����� ��������� �� ����������

		work->startGenerator(counter+1); // ������ ���������� ���������.
		while(counter) // ������������ �������� ����������
		{
			work->adjustSounding(curFrq);

			// ������������� ������� ������������ ������.
			work->cleanLineAccumulator();
			for (unsigned k = 0; k < conf.getPulseCount(); k++) // ������� ������ ������������ �� ����� �������
			{
				work->ASYNC_TRANSFER(); // �������� ���
				
				// ���� �������� �� ��������� ����������� � ������.
				// READ_BUFISCOMPLETE - ����� �� ������� 47 ��
				while(work->READ_ISCOMPLETE(msTimeout) == NULL);

				// ��������� ���
				work->READ_ABORTIO();					

				work->accumulateLine(curFrq); // ������� ����� ��� ���������� �������
				counter--; // ���������� � ��������� ���������� ��������
			}
			// �������� �� ���������� ��������� ������������ �� ����� �������
			if(conf.getPulseCount() > 1)
				work->averageLine(conf.getPulseCount()); 
			// �������� ������ �� char (����� �� 6 ���) � ���������� ����� � �����.
			switch(conf.getVersion())
			{
			case 0: // ���
				work->saveLine(curFrq);
				break;
			case 1: // ��� ������
				work->saveDirtyLine();
				break;
			case 2: // ��� ����������
				work->saveFullData();
				break;
			case 3: // CDF

				break;
			}

			curFrq += ion.fstep; // ��������� ������� ������������
		}

		// �������� ���������� � ����������� ����������� �������
		std::vector<std::string> log = work->getLog();
		std::ofstream output_file("parus.log");
		std::ostream_iterator<std::string> output_iterator(output_file, "\n");
		std::copy(log.begin(), log.end(), output_iterator);

		delete work;
	}
	catch(std::exception &e)
	{
		std::cerr << std::endl;
		std::cerr << "���������: " << e.what() << std::endl;
		std::cerr << "���      : " << typeid(e).name() << std::endl;
		RetStatus = -1;
	}
	Beep( 1500, 300 );

	return RetStatus;
}

void SetPriorityClass(void){
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