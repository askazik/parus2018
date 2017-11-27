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
    // 1. Читаем файл конфигурации для измерения ионограммы.
    // ===========================================================================================
	xmlconfig conf;
	ionHeaderNew2 _header = conf.getIonogramHeader();

	std::cout << "Используем конфигурационный файл: <" << conf.getFileName() << ">." << std::endl;

	std::cout << "Параметры зондирования: " << std::endl;
	std::cout << "Частоты: " << _header.freq_min << " кГц - " << _header.freq_max 
				<< " кГц, усиление = " << conf.getGain() 
				<< " дБ, аттенюатор = " << conf.getAttenuation() << " выкл(0)/вкл(1)." << std::endl;

    // ===========================================================================================
    // 1. Читаем файл конфигурации для амплитудных измерений.
    // ===========================================================================================
	//xmlconfig conf("config.xml", AMPLITUDES);

	//std::cout << "Используем конфигурационный файл: <" << conf.getFileName() << ">." << std::endl;

	//std::cout << "Параметры зондирования: " << std::endl;
	//std::cout << "Усиление = " << conf.getGain() 
	//			<< " дБ, аттенюатор = " << conf.getAttenuation() << " выкл(0)/вкл(1)." << std::endl;
	//for(size_t i = 0; i < conf.getModulesCount(); i++)
	//	std::cout << conf.getAmplitudesFrq(i) << " кГц" << std::endl;

    // ===========================================================================================
    // 2. Конфигурирование сеанса.
    // ===========================================================================================
	int RetStatus = 0;
	try	
	{
		// Подготовка аппаратуры к зондированию.
		// Открытие выходнго файла данных и запись заголовка.
		ionogramSettings ion = conf.getIonogramSettings();
		parusWork *work = new parusWork(&conf);
		// Задержка для корректной инициализации ADC
		Sleep(30*1000); // полминуты

		DWORD msTimeout = 25;
		unsigned short curFrq = ion.fbeg; // текущая частота зондирования, кГц
		int counter = ion.count * conf.getPulseCount(); // число импульсов от генератора

		work->startGenerator(counter+1); // Запуск генератора импульсов.
		while(counter) // обрабатываем импульсы генератора
		{
			work->adjustSounding(curFrq);

			// Инициализация массива суммирования нулями.
			work->cleanLineAccumulator();
			for (unsigned k = 0; k < conf.getPulseCount(); k++) // счётчик циклов суммирования на одной частоте
			{
				work->ASYNC_TRANSFER(); // запустим АЦП
				
				// Цикл проверки до появления результатов в буфере.
				// READ_BUFISCOMPLETE - сбоит на частоте 47 Гц
				while(work->READ_ISCOMPLETE(msTimeout) == NULL);

				// Остановим АЦП
				work->READ_ABORTIO();					

				work->accumulateLine(curFrq); // частота нужна для заполнения журнала
				counter--; // приступаем к обработке следующего импульса
			}
			// усредним по количеству импульсов зондирования на одной частоте
			if(conf.getPulseCount() > 1)
				work->averageLine(conf.getPulseCount()); 
			// Усечение данных до char (сдвиг на 6 бит) и сохранение линии в файле.
			switch(conf.getVersion())
			{
			case 0: // ИПГ
				work->saveLine(curFrq);
				break;
			case 1: // без потерь
				work->saveDirtyLine();
				break;
			case 2: // для калибровки
				work->saveFullData();
				break;
			case 3: // CDF

				break;
			}

			curFrq += ion.fstep; // следующая частота зондирования
		}

		// Сохраним информацию о наступлении ограничения сигнала
		std::vector<std::string> log = work->getLog();
		std::ofstream output_file("parus.log");
		std::ostream_iterator<std::string> output_iterator(output_file, "\n");
		std::copy(log.begin(), log.end(), output_iterator);

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