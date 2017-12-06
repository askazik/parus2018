// ===========================================================================
// Запись ионограммы в файл.
// ===========================================================================
#include "ionogram.h"

using namespace parus;

int main(void)
{	
	setlocale(LC_ALL,"Russian"); // настройка локали на вывод сообщений по-русски
	SetPriorityClass();

    // ===========================================================================================
    // 1. Читаем файл конфигурации для сеанса.
    // ===========================================================================================
	xml_project project;
	xml_ionogram ionogram;
	xml_amplitudes amplitudes;

	ionHeaderNew2 _header = ionogram.getIonogramHeader();

	std::cout << "Используем конфигурационный файл: <" << project.getFileName() << ">." << "\n\n";

	std::cout << "Параметры ионограммы: " << std::endl;
	std::cout << "Частоты: " << _header.freq_min << " кГц - " << _header.freq_max 
				<< " кГц, усиление = " << ionogram.getGain() 
				<< " дБ, аттенюатор = " << ionogram.getAttenuation() << " выкл(0)/вкл(1)." << "\n\n";

	std::cout << "Параметры амплитудных измерений: " << std::endl;
	std::cout << "Усиление = " << amplitudes.getGain() 
				<< " дБ, аттенюатор = " << amplitudes.getAttenuation() << " выкл(0)/вкл(1)." << std::endl;
	for(size_t i = 0; i < amplitudes.getModulesCount(); i++)
		std::cout << amplitudes.getAmplitudesFrq(i) << " кГц" << std::endl;

    // ===========================================================================================
    // 2. Конфигурирование и исполнение сеанса.
    // ===========================================================================================
	int RetStatus = 0;
	try	
	{
		parusWork *work = new parusWork(); // Подготовка аппаратуры к зондированию.
		Sleep(500); // Задержка для корректной инициализации.	

		// Перебор по элементам проекта.
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
				std::cout << "Неизвестный блок измерений <" << project.getMeasurement() << "> в конфигурационном файле." << std::endl;
			}
		}

		delete work;
	}
	catch(std::exception &e)
	{
		std::cerr << std::endl;
		std::cerr << "Сообщение: " << e.what() << std::endl;
		std::cerr << "Тип      : " << typeid(e).name() << std::endl;
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