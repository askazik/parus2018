// ===========================================================================
// ������ ���������� � ����.
// ===========================================================================
#include "ionogram.h"

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

	std::cout << "��������� ����������: " << std::endl;
	std::cout << "�������: " << _header.freq_min << " ��� - " << _header.freq_max 
				<< " ���, �������� = " << ionogram.getGain() 
				<< " ��, ���������� = " << ionogram.getAttenuation() << " ����(0)/���(1)." << "\n\n";

	std::cout << "��������� ����������� ���������: " << std::endl;
	std::cout << "�������� = " << amplitudes.getGain() 
				<< " ��, ���������� = " << amplitudes.getAttenuation() << " ����(0)/���(1)." << std::endl;
	for(size_t i = 0; i < amplitudes.getModulesCount(); i++)
		std::cout << amplitudes.getAmplitudesFrq(i) << " ���" << std::endl;

    // ===========================================================================================
    // 2. ���������������� � ���������� ������.
    // ===========================================================================================
	int RetStatus = 0;
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
				RetStatus = work->ionogram(&ionogram);
				break;
			case AMPLITUDES:
				RetStatus = work->amplitudes(&amplitudes);
				break;
			default:
				std::cout << "����������� ���� ��������� <" << project.getMeasurement() << "> � ���������������� �����." << std::endl;
			}
		}

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