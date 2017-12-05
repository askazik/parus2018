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
    // 1. ������ ���� ������������ ��� ������.
    // ===========================================================================================
	xml_project project;
	xml_ionogram ionogram;
	xml_amplitudes amplitudes;

	ionHeaderNew2 _header = ionogram.getIonogramHeader();

	std::cout << "���������� ���������������� ����: <" << project.getFileName() << ">." << "\n\n";

	std::cout << "��������� ����������: " << std::endl;
	std::cout << "�������: " << _header.freq_min << " ��� - " << _header.freq_max 
				<< " ���, �������� = " << ionogram.getGain() 
				<< " ��, ���������� = " << ionogram.getAttenuation() << " ����(0)/���(1)." << "\n\n";

	std::cout << "��������� ����������� ���������: " << std::endl;
	std::cout << "�������� = " << amplitudes.getGain() 
				<< " ��, ���������� = " << amplitudes.getAttenuation() << " ����(0)/���(1)." << std::endl;
	for(int i = 0; i < amplitudes.getModulesCount(); i++)
		std::cout << amplitudes.getAmplitudesFrq(i) << " ���" << std::endl;

    // ===========================================================================================
    // 2. ���������������� � ���������� ������.
    // ===========================================================================================
	int RetStatus = 0;
	DWORD msTimeout = 25;
	unsigned short curFrq; // ������� ������� ������������, ���
	int counter; // ����� ��������� �� ����������

	try	
	{
		parusWork *work = new parusWork(); // ���������� ���������� � ������������.
		Sleep(500); // �������� ��� ���������� �������������.	

		// ������� �� ��������� �������.
		for(size_t i=0; i < project.getModulesCount(); i++)
		{
			switch(project.getMeasurement())
			{
			case IONOGRAM:
				work->setup(&ionogram);
				curFrq = ionogram.getModule(0)._map.at("fbeg");
				counter = ion.count * ionogram.getPulseCount(); // ����� ��������� �� ����������
				work->startGenerator(counter+1); // ������ ���������� ���������.

				break;
			case AMPLITUDES:
				work->setup(&amplitudes);
				break;
			default:
				std::cout << "����������� ���� ��������� <" << project.getMeasurement() << "> � ���������������� �����." << std::endl;
			}
		}

		

		while(counter) // ������������ �������� ����������
		{
			work->adjustSounding(curFrq);

			// ������������� ������� ������������ ������.
			work->cleanLineAccumulator();
			for (unsigned k = 0; k < ionogram.getPulseCount(); k++) // ������� ������ ������������ �� ����� �������
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
			if(ionogram.getPulseCount() > 1)
				work->averageLine(ionogram.getPulseCount()); 
			// �������� ������ �� char (����� �� 6 ���) � ���������� ����� � �����.
			switch(ionogram.getVersion())
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