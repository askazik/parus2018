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
		std::map<std::string, unsigned> _map; // сохраняем пару ключ/значение

		void fill(const XML::XMLElement *parent)
		{
			_name = (parent->Attribute("name")) ? std::string(parent->Attribute("name")) : "";

			const XML::XMLElement *element;
			for (
				element = parent->FirstChildElement();
				element;
				element = element->NextSiblingElement()
			)
			{
				std::string name = element->Name();
				_map.insert(std::pair<std::string, unsigned>(name,element->IntText(0)));
			}
		}
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
		struct tm getUTC(void);

		static const char* MeasurementNames[];
	public:
		xml_unit(Measurement mes = MEASUREMENT, std::string fullName = XML_CONFIG_DEFAULT_FILE_NAME);
		std::string getFileName(void){return _fullFileName;}
		const XML::XMLElement *getXMLModule(int i){return _xml_modules[i];}
		const unit getHeader(void){return _header;}
		const unit getModule(int i){return _modules[i];}
		int getModulesCount(void){return _xml_modules.size();}
		Measurement getMeasurement(void){return _measurement;}

		// Информация из заголовка.
		virtual unsigned getVersion(void){return _header._map.at("version");}
		virtual unsigned getHeightStep(void){return _header._map.at("height_step");}
		virtual void setHeightStep(double value){_header._map.at("height_step") = static_cast<unsigned>(value);}
		virtual unsigned getHeightCount(void){return _header._map.at("height_count");}
		virtual void setHeightCount(unsigned value){_header._map.at("height_count") = value;}
		virtual unsigned getPulseCount(void){return _header._map.at("pulse_count");}
		virtual void setPulseCount(unsigned value){_header._map.at("pulse_count") = value;}
		virtual unsigned getAttenuation(void){return _header._map.at("attenuation");}
		virtual unsigned getGain(void){return _header._map.at("gain");}
		virtual unsigned getPulseFrq(void){return _header._map.at("pulse_frq");}
		virtual unsigned getPulseDuration(void){return _header._map.at("pulse_duration");}
		virtual unsigned getSwitchFrequency(void){return _header._map.at("switch_frequency");}	
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
		ionHeaderNew2 getIonogramHeader(void);
		virtual unsigned getFrequenciesCount(void);
	};

	// Блок амплитуд
	class xml_amplitudes : public xml_unit 
	{
	public:
		xml_amplitudes(std::string fullName = XML_CONFIG_DEFAULT_FILE_NAME) : xml_unit(AMPLITUDES, fullName){}
		unsigned getAmplitudesFrq(unsigned i){return (getModulesCount()) ? getModule(i)._map.at("frequency") : 0;}

		// Формирование заголовка файла
		dataHeader getAmplitudesHeader(void);
	};
}; // end namespace parus

#endif // __CONFIG_H__