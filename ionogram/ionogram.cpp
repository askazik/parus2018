// ===========================================================================
// ������ ���������� � ����.
// ===========================================================================
#include "ionogram.h"

using namespace parus;

int main(void)
{	
	setlocale(LC_ALL,"Russian"); // ��������� ������ �� ����� ��������� ��-������
	mySetPriorityClass();

	try	
	{
		CADCOverflowException e = CADCOverflowException(15000, true, false);
		CFrequencyException ee(e, 5600);
		throw ee;
	}
	catch(CParusException &e)
	{
		std::cerr << std::endl;
			std::cerr << "���������   : " << e.what() << std::endl;
			std::cerr << "���         : " << typeid(e).name() << std::endl;

		if(typeid(e) == typeid(CADCOverflowException))
		{
			CADCOverflowException ee = dynamic_cast<CADCOverflowException &>(e);
			std::cerr << "������, ��  : " << ee.getHeight() << std::endl;
			std::cerr << "������������: Re = " << std::boolalpha << ee.getOverflowRe() << ", Im = " << ee.getOverflowIm() << "." << std::endl;
		}

		if(typeid(e) == typeid(CFrequencyException))
		{
			CFrequencyException ee = dynamic_cast<CFrequencyException &>(e);
			std::cerr << "������, ��  : " << ee.getHeight() << std::endl;
			std::cerr << "������������: Re = " << std::boolalpha << ee.getOverflowRe() << ", Im = " << ee.getOverflowIm() << "." << std::endl;
			std::cerr << "�������, ���: " << ee.getFrequency() << std::endl;
		}
	}

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
	int RetStatus = -1; // ��������� ���������� ���������
	try	
	{
		parusWork *work = new parusWork(); // ���������� ���������� � ������������.
		Sleep(500); // �������� ��� ���������� �������������.	

		// ������� �� ��������� �������.
		for(size_t i=0; i < project.getModulesCount(); i++)
		{
			unit tmp = project.getModule(i);
			switch(tmp._meas)
			{
			case MEASUREMENT: // ���������� �.�. ����������� �������� ���������
				break;
			case IONOGRAM:
				RetStatus = work->ionogram(&ionogram);
				break;
			case AMPLITUDES:
				RetStatus = work->amplitudes(&amplitudes);
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