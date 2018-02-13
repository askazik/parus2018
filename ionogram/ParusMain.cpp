// ===========================================================================
// ������ ���������� � ����.
// ===========================================================================
#include "ParusMain.h"

using namespace parus;

int main(void)
{	
	setlocale(LC_ALL,"Russian"); // ��������� ������ �� ����� ��������� ��-������
	mySetPriorityClass();

    // ===========================================================================================
    // 1. ������ ���� ������������ ��� ������.
    // ===========================================================================================
	xml_project project;
	xml_ionogram ionogram;
	xml_amplitudes amplitudes;

	ionHeaderNew2 _header = ionogram.getIonogramHeader();

	std::cout << "���������� ���������������� ����: <" << project.getFileName() << ">." << "\n\n";
	unsigned dt = project.getModule(1)._map.at("dt");

    // ===========================================================================================
    // 2. ���������������� � ���������� ������.
    // ===========================================================================================
	int RetStatus = -1; // ��������� ���������� ���������
	try	
	{
		parusWork *work = new parusWork(); // ���������� ���������� � ������������.

		// ��������� �������� ���� ��� ��� (���������� ��� ����������� "������").
		std::vector<double> zero_shift;
		zero_shift.push_back(65);
		zero_shift.push_back(112);
		work->adjustZeroShift(zero_shift);

		Sleep(1000);

		// ������� �� ��������� �������.
		for(size_t i=0; i < project.getModulesCount(); i++)
		{
			unit tmp = project.getModule(i);
			switch(tmp._meas)
			{
			case MEASUREMENT: // ���������� �.�. ����������� �������� ���������
				break;
			case IONOGRAM:
				// ���������
				std::cout << "��������� ����������: " << std::endl;
				std::cout << "�������: " << _header.freq_min << " ��� - " 
					<< _header.freq_max << " ���, �������� = " 
					<< ionogram.getGain() << " ��, ���������� = " 
					<< ionogram.getAttenuation() << " ����(0)/���(1)." << "\n\n";

				work->setup(&ionogram);
				work->openIonogramFile(&ionogram);
				RetStatus = work->ionogram(&ionogram);
				work->closeOutputFile();
				Beep( 500, 600 );
				break;
			case AMPLITUDES:
				// ���������
				std::cout << "��������� ����������� ���������: " << std::endl;
				std::cout << "�������� = " << amplitudes.getGain() 
					<< " ��, ���������� = " << amplitudes.getAttenuation() 
					<< " ����(0)/���(1)." << std::endl;
				for(size_t i = 0; i < amplitudes.getModulesCount(); i++)
					std::cout << amplitudes.getAmplitudesFrq(i) 
					<< " ���" << std::endl;
				std::cout << "����� ��������� ��������: " << dt << " �.\n";

				work->setup(&amplitudes);
				work->openDataFile(&amplitudes);
				RetStatus = work->amplitudes(&amplitudes,dt);
				work->closeOutputFile();
				break;
			default:
				std::cerr << "� ���������������� ����� ���������� ����������� ���� ���������." << std::endl;
			}
		}

		delete work;
	}
	catch(std::exception &e)
	{
		std::cerr << std::endl;
		std::cerr << "���������: " << e.what() << std::endl;
		std::cerr << "���      : " << typeid(e).name() << std::endl;
	}
	Beep( 1500, 300 );

	return RetStatus;
}