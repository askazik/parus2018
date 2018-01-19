// ===========================================================================
// Заголовочный файл для работы с конфигурационными файлами.
// ===========================================================================
#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdexcept>
#include <string>
#include <vector>
#include <map>
#include <iterator>
#include <iostream>
#include <fstream>
#include <ctime>

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
		ionHeaderNew2(void) : 
			ver(1),
			height_min(0), // Это не означает, что зондирование от поверхности. Есть задержки!!!
			height_step(0),
			count_height(0),
			switch_frequency(0),
			freq_min(0),
			freq_max(0),    
			count_freq(0),
			count_modules(0)
		{
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
	// Конфигурационный файл
	// ===========================================================================
	enum  Measurement { MEASUREMENT, IONOGRAM, AMPLITUDES}; // перечисление для вариантов проведения эксперимента

	#pragma pack(pop)

	// Блок элементарного модуля (header, module, ...)
	struct unit
	{
		std::string _name;
		Measurement _meas;
		std::map<std::string, unsigned> _map; // сохраняем пару ключ/значение
		void fill(const XML::XMLElement *parent);
	};

	// Базовый блок конфигурационного xml-файла
	class xml_unit 
	{
	protected:
		// xml
		XML::XMLDocument _document;
		const XML::XMLElement *_xml_root;
		const XML::XMLElement *_xml_measurement;
		const XML::XMLElement *_xml_header;
		std::vector<const XML::XMLElement *> _xml_modules;
		std::string _fullFileName;
		Measurement _measurement;

		// measurement
		unit _header;
		std::vector<unit> _modules;

		void findMeasurement(Measurement mes);
		const struct tm getUTC();
	public:
		static const char* MeasurementNames[];

		xml_unit(Measurement mes = MEASUREMENT, std::string fullName = XML_CONFIG_DEFAULT_FILE_NAME);
		std::string getFileName(){return _fullFileName;}
		const XML::XMLElement *getXMLModule(int i){return _xml_modules[i];}
		const unit getHeader(){return _header;}
		const unit getModule(int i){return _modules[i];}
		const unsigned getModulesCount(){return _xml_modules.size();}
		const Measurement getMeasurement(){return _measurement;}

		// Информация из заголовка.
		virtual const unsigned getVersion() const {return _header._map.at("version");}
		virtual const unsigned getHeightStep() const {return _header._map.at("height_step");}
		virtual void setHeightStep(double value){_header._map.at("height_step") = static_cast<unsigned>(value);}
		virtual const unsigned getHeightCount() const {return _header._map.at("height_count");}
		virtual void setHeightCount(unsigned value){_header._map.at("height_count") = value;}
		virtual const unsigned getPulseCount() const {return _header._map.at("pulse_count");}
		virtual void setPulseCount(unsigned value){_header._map.at("pulse_count") = value;}
		virtual const unsigned getAttenuation() const {return _header._map.at("attenuation");}
		virtual const unsigned getGain() const {return _header._map.at("gain");}
		virtual const unsigned getPulseFrq() const {return _header._map.at("pulse_frq");}
		virtual const unsigned getPulseDuration() const {return _header._map.at("pulse_duration");}
		virtual const unsigned getSwitchFrequency() const {return _header._map.at("switch_frequency");}	
	};

	// Блок планирования эксперимента
	class xml_project : public xml_unit 
	{
	public:
		xml_project(std::string fullName = XML_CONFIG_DEFAULT_FILE_NAME) : xml_unit(MEASUREMENT, fullName){}
	};

	// Блок ионограммы
	class xml_ionogram : public xml_unit 
	{
	public:
		xml_ionogram(std::string fullName = XML_CONFIG_DEFAULT_FILE_NAME) : xml_unit(IONOGRAM, fullName){}

		// Формирование заголовка файла
		const ionHeaderNew2 getIonogramHeader();
		virtual const unsigned getFrequenciesCount();
	};

	// Блок амплитуд
	class xml_amplitudes : public xml_unit 
	{
	public:
		xml_amplitudes(std::string fullName = XML_CONFIG_DEFAULT_FILE_NAME) : xml_unit(AMPLITUDES, fullName){}
		const unsigned getAmplitudesFrq(unsigned i){return (getModulesCount()) ? getModule(i)._map.at("frequency") : 0;}

		// Формирование заголовка файла
		const dataHeader getAmplitudesHeader();
	};
}; // end namespace parus

#endif // __CONFIG_H__