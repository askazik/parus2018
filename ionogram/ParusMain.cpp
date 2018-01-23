// ===========================================================================
// Запись ионограммы в файл.
// ===========================================================================
#include "ParusMain.h"

using namespace parus;

int main(void)
{	
	setlocale(LC_ALL,"Russian"); // настройка локали на вывод сообщений по-русски
	mySetPriorityClass();

    // ===========================================================================================
    // 1. Читаем файл конфигурации для сеанса.
    // ===========================================================================================
	xml_project project;
	xml_ionogram ionogram;
	xml_amplitudes amplitudes;

	ionHeaderNew2 _header = ionogram.getIonogramHeader();

	std::cout << "Используем конфигурационный файл: <" << project.getFileName() << ">." << "\n\n";
	unsigned dt = project.getModule(1)._map.at("dt");

	std::cout << "Параметры ионограммы: " << std::endl;
	std::cout << "Частоты: " << _header.freq_min << " кГц - " << _header.freq_max 
				<< " кГц, усиление = " << ionogram.getGain() 
				<< " дБ, аттенюатор = " << ionogram.getAttenuation() << " выкл(0)/вкл(1)." << "\n\n";

	std::cout << "Параметры амплитудных измерений: " << std::endl;
	std::cout << "Усиление = " << amplitudes.getGain() 
				<< " дБ, аттенюатор = " << amplitudes.getAttenuation() << " выкл(0)/вкл(1)." << std::endl;
	for(size_t i = 0; i < amplitudes.getModulesCount(); i++)
		std::cout << amplitudes.getAmplitudesFrq(i) << " кГц" << std::endl;
	std::cout << "Время измерения амплитуд: " << dt << " с.\n";

    // ===========================================================================================
    // 2. Конфигурирование и исполнение сеанса.
    // ===========================================================================================
	int RetStatus = -1; // нештатное завершение программы
	try	
	{
		parusWork *work = new parusWork(); // Подготовка аппаратуры к зондированию.

		// Перебор по элементам проекта.
		for(size_t i=0; i < project.getModulesCount(); i++)
		{
			unit tmp = project.getModule(i);
			switch(tmp._meas)
			{
			case MEASUREMENT: // пропускаем т.к. отсутствует реальное измерение
				break;
			case IONOGRAM:
				work->setup(&ionogram);
				work->openIonogramFile(&ionogram);
				RetStatus = work->ionogram(&ionogram);
				work->closeOutputFile();
				break;
			case AMPLITUDES:
				work->setup(&amplitudes);
				work->openDataFile(&amplitudes);
				RetStatus = work->amplitudes(&amplitudes,dt);
				work->closeOutputFile();
				break;
			default:
				std::cerr << "В конфигурационном файле встретился неизвестный блок измерений." << std::endl;
			}
		}

		delete work;
	}
	catch(std::exception &e)
	{
		std::cerr << std::endl;
		std::cerr << "Сообщение: " << e.what() << std::endl;
		std::cerr << "Тип      : " << typeid(e).name() << std::endl;
	}
	Beep( 1500, 300 );

	return RetStatus;
}