// ===========================================================================
// Заголовочный файл для работы с конфигурационными файлами.
// ===========================================================================
#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdexcept>
#include <string>
#include <vector>
#include <iterator>
#include <iostream>
#include <fstream>
#include <ctime>

#include "config.h"
#include "tinyxml2.h"

namespace XML = tinyxml2;

#define XML_CONFIG_DEFAULT_FILE_NAME "config.xml"

namespace parus {

	#pragma pack(push, 1)

	// ===========================================================================
	// Структура для измерения ионограмм
	// ===========================================================================
	struct ionHeaderNew2 { 	    // === Заголовок файла ионограмм ===
		unsigned ver; // номер версии
		struct tm time_sound; // GMT время получения ионограммы
		unsigned height_min; // начальная высота, м
		unsigned height_step; // шаг по высоте, м
		unsigned count_height; // число высот
		unsigned switch_frequency; // частота переключения антенн ионозонда 
		unsigned freq_min; // начальная частота, кГц (первого модуля)
		unsigned freq_max; // конечная частота, кГц (последнего модуля)   
		unsigned count_freq; // число частот во всех модулях
		unsigned count_modules; // количество модулей зондирования

		// Начальная инициализация структуры.
		ionHeaderNew2(void)
		{
			ver = 2;
			height_min = 0; // Это не означает, что зондирование от поверхности. Есть задержки!!!
			height_step = 0;
			count_height = 0;
			switch_frequency = 0;
			freq_min = 0;
			freq_max = 0;    
			count_freq = 0;
			count_modules = 0;
		}
	};

	struct dataHeader { 	    // === Заголовок файла данных ===
		unsigned ver; // номер версии
		struct tm time_sound; // GMT время получения зондирования
		unsigned height_min; // начальная высота, м (всё, что ниже при обработке отбрасывается)
		unsigned height_max; // конечная высота, м (всё, что выше при обработке отбрасывается)
		unsigned height_step; // шаг по высоте, м (реальный шаг, вычисленный по частоте АЦП)
		unsigned count_height; // число высот (размер исходного буфера АЦП при зондировании, fifo нашего АЦП 4Кб. Т.е. не больше 1024 отсчётов для двух квадратурных каналов)
		unsigned count_modules; // количество модулей/частот зондирования
		unsigned pulse_frq; // частота зондирующих импульсов, Гц
	};

	// ===========================================================================
	// Модуль для амплитудных измерений
	// ===========================================================================
	struct myModule {  
		unsigned frq;   // частота модуля, кГц
	};

	// ===========================================================================
	// Конфигурационный файл
	// ===========================================================================
	enum  Measurement { IONOGRAM, AMPLITUDES }; // перечисление для вариантов проведения эксперимента

	// Общие параметры для программирования устройства.
	struct ionosounder { 
		unsigned ver; // номер версии для сохранения файла результатов
		unsigned height_step; // шаг по высоте
		unsigned height_count; // количество высот
		unsigned pulse_count; // импульсов зондирования на каждой частоте
		unsigned attenuation; // ослабление (аттенюатор) 1/0 = вкл/выкл
		unsigned gain;	// усиление, дБ (g = value/6, 6дБ = приращение в 4 раза по мощности)
		unsigned pulse_frq; // частота зондирующих импульсов, Гц
		unsigned pulse_duration; // длительность зондирующих импульсов, мкс
		unsigned switch_frequency; // Частота переключения антенн, кГц (не используется)
		unsigned modules_count; // Число модулей
	};

	struct ionogramSettings {  
		unsigned fstep;  // шаг по частоте ионограммы, кГц
		unsigned fbeg;   // начальная частота модуля, кГц
		unsigned fend;   // конечная частота модуля, кГц
		unsigned count;  // количество частот зондирования
	};

	#pragma pack(pop)

	// Общий блок конфигурационного xml-файла
	class xmlconfig {

	protected:
		ionosounder _device;
		std::string _fullFileName;
		Measurement _mes;
		XML::XMLDocument _document;

		// Структура для измерения ионограмм
		ionogramSettings _ionogram;
		// Вектор структур для амплитудных измерений
		std::vector<myModule> _amplitudes;

		void loadMeasurementHeader(const XML::XMLElement *xml_mes);
		void loadIonogramConfig(const XML::XMLElement *xml_mes);
		void loadAmplitudesConfig(const XML::XMLElement *xml_mes);

	public:
		xmlconfig(std::string fullName = XML_CONFIG_DEFAULT_FILE_NAME, Measurement mes = IONOGRAM);

		std::string getFileName(void){return _fullFileName;}

		unsigned getVersion(void){return _device.ver;}
		unsigned getHeightStep(void){return _device.height_step;}
		void setHeightStep(double value){_device.height_step = static_cast<unsigned>(value);}
		unsigned getHeightCount(void){return _device.height_count;}
		void setHeightCount(unsigned value){_device.height_count = value;}
		unsigned getModulesCount(void){return _device.modules_count;}
		unsigned getPulseCount(void){return _device.pulse_count;}
		void setPulseCount(unsigned value){_device.pulse_count = value;}
		unsigned getAttenuation(void){return _device.attenuation;}
		unsigned getGain(void){return _device.gain;}
		unsigned getPulseFrq(void){return _device.pulse_frq;}
		unsigned getPulseDuration(void){return _device.pulse_duration;}
		unsigned getSwitchFrequency(void){return _device.switch_frequency;}
		unsigned getMeasurement(void){return _mes;}

		unsigned getAmplitudesFrq(unsigned i){return (getModulesCount()) ? _amplitudes[i].frq : 0;}
		ionogramSettings getIonogramSettings(void){return _ionogram;}

		// Формирование заголовка файла
		ionHeaderNew2 getIonogramHeader(void);
		dataHeader getAmplitudesHeader(void);
	};


	//// ===========================================================================
	//// Амплитуды
	//// ===========================================================================
	//struct myModule {  
	//	unsigned frq;   // частота модуля, кГц
	//};

	//// Класс доступа к конфигурационному файлу зондирования.
	//class parusConfig  : public config {
 //   
	//	unsigned _ver; // номер версии
	//	unsigned _height_step; // шаг по высоте, м
	//	unsigned _height_count; // количество высот
	//	unsigned _modules_count; // количество модулей/частот зондирования
	//	unsigned _pulse_count; // импульсов зондирования на каждой частоте
	//	unsigned _attenuation; // ослабление (аттенюатор) 1/0 = вкл/выкл
	//	unsigned _gain;	// усиление (g = value/6, 6дБ = приращение в 4 раза по мощности)
	//	unsigned _pulse_frq; // частота зондирующих импульсов, Гц
	//	unsigned _pulse_duration; // длительность зондирующих импульсов, мкс
	//	unsigned _height_min; // начальная высота, км (всё, что ниже при обработке отбрасывается)
	//	unsigned _height_max; // конечная высота, км (всё, что выше при обработке отбрасывается)

	//	myModule	*_ptModules; // указатель на массив модулей

	//	void getModulesCount(std::ifstream &fin);
	//	void getModules(std::ifstream &fin);
	//	myModule getCurrentModule(std::ifstream &fin);
	//public:
	//	parusConfig(std::string fullName);
	//	~parusConfig(void);

	//	void loadConf(std::string fullName);
	//	int getModulesCount(void){return _modules_count;}
	//	unsigned getHeightStep(void){return _height_step;}
	//	void setHeightStep(double value){_height_step = static_cast<unsigned>(value);}
	//	unsigned getHeightCount(void){return _height_count;}
	//	unsigned getVersion(void){return _ver;}
	//	unsigned getFreq(int num){return _ptModules[num].frq;}

	//	unsigned getAttenuation(void){return _attenuation;} // ослабление (аттенюатор) 1/0 = вкл/выкл
	//	unsigned getGain(void){return _gain;}	// усиление (g = value/6, 6дБ = приращение в 4 раза по мощности)
	//	unsigned getPulse_frq(void){return _pulse_frq;} // частота зондирующих импульсов, Гц
	//	unsigned getPulse_duration(void){return _pulse_duration;} // длительность зондирующих импульсов, мкс

	//	unsigned getHeightMin(void){return _height_min;} // начальная высота, км (всё, что ниже при обработке отбрасывается)
	//	unsigned getHeightMax(void){return _height_max;} // конечная высота, км (всё, что выше при обработке отбрасывается)
	//};

}; // end namespace parus

#endif // __CONFIG_H__