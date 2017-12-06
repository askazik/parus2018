// ===========================================================================
// Операции с конфигурационным файлом зондирования.
// ===========================================================================
#include "config.h"

/// <summary>
/// Содержит классы и определения для работы с конфигурацией, сохранённой в xml-файле.
/// </summary>
namespace parus {

	// ===========================================================================
	// Конфигурационный файл XML
	// ===========================================================================
	const char* xml_unit::MeasurementNames[] = {"measurement", "ionogram", "amplitudes" }; // имена блоков измерений

	/// <summary>
	/// Initializes a new instance of the base <see cref="xmlunit"/> class.
	/// </summary>
	/// <param name="fullName">The full file name.</param>
	/// <param name="mes">The measurement type.</param>
	xml_unit::xml_unit(Measurement mes, std::string fullName)
	{
		_measurement = mes;
		_fullFileName = fullName;

		// Откроем конфигурационный файл.
		XML::XMLError eResult = _document.LoadFile(fullName.c_str());
		if (eResult != tinyxml2::XML_SUCCESS) 
			throw std::runtime_error("Ошибка открытия конфигурационного файла <" + fullName + ">.");

		// Получим родительский элемент.
		_xml_root = _document.RootElement();
		if(_xml_root == nullptr)
			throw std::runtime_error("В конфигурационном файле отсутствует корневой элемент <parus>.");

		// Находим заголовок и модули измерения.
		findMeasurement(mes);
		
		// Заполним header
		_xml_header = _xml_measurement->FirstChildElement("header");
		_header.fill(_xml_header);
		if(_xml_header == nullptr)
		{
			std::string tmp(MeasurementNames[mes]);
			throw std::runtime_error("В блоке измерения <" + tmp + "> отсутствует элемент <header>.");
		}
		
		// Заполним имеющимися module
		const XML::XMLElement *module = _xml_measurement->FirstChildElement("module");
		if(module == nullptr)
		{
			std::string tmp(MeasurementNames[mes]);
			throw std::runtime_error("В блоке измерения <" + tmp + "> отсутствует элемент <module>.");
		}
		while (module != nullptr)
		{
			unit tmp;
			tmp.fill(module);

			_xml_modules.push_back(module);
			_modules.push_back(tmp);
			module = module->NextSiblingElement("module");
		}
	}

	/// <summary>
	/// Gets the measurement block.
	/// </summary>
	/// <param name="mes">The measurement type.</param>
	/// <returns>XMLElement* - measurement element.</returns>
	void xml_unit::findMeasurement(Measurement mes)
	{
		for (
         _xml_measurement = _xml_root->FirstChildElement("Measurement");
         _xml_measurement;
         _xml_measurement = _xml_measurement->NextSiblingElement()
        ) 
		{
			if(!strcmp(_xml_measurement->Attribute("name"), MeasurementNames[mes]))
				break;
		}
	}

	struct tm xml_unit::getUTC(void)
	{
	// Obtain coordinated universal time (!!!! UTC !!!!!):
    // ==================================================================================================
    // The value returned generally represents the number of seconds since 00:00 hours, Jan 1, 1970 UTC
    // (i.e., the current unix timestamp). Although libraries may use a different representation of time:
    // Portable programs should not use the value returned by this function directly, but always rely on
    // calls to other elements of the standard library to translate them to portable types (such as
    // localtime, gmtime or difftime).
    // ==================================================================================================
		time_t ltime;
		time(&ltime);
		struct tm newtime;
		gmtime_s(&newtime, &ltime);
		return newtime;
	}

	/// <summary>
	/// Формирование заголовка файла ионограмм.
	/// </summary>
	/// <returns>Структура ionHeaderNew2 для записи в заголовок файла.</returns>
	ionHeaderNew2 xml_ionogram::getIonogramHeader(void)
	{
		ionHeaderNew2 _out;
		unit module0 = getModule(0);
		struct tm newtime = getUTC();

		// Заполнение заголовка файла.
		_out.time_sound = newtime;
		_out.count_height = getHeightCount();
		_out.count_modules = getModulesCount();
		_out.freq_max = module0._map.at("fend");
		_out.freq_min = module0._map.at("fbeg");
		_out.count_freq = getFrequenciesCount();
		_out.height_min = 0;
		_out.height_step = getHeightStep();
		_out.switch_frequency = getSwitchFrequency();
		_out.ver = getVersion();

		return _out;
	}

	/// <summary>
	/// Возвращает количество частот зондирования ионограммы.
	/// </summary>
	/// <returns></returns>
	unsigned xml_ionogram::getFrequenciesCount(void)
	{
		unit module0 = getModule(0);
		return 1 + (module0._map.at("fend") - module0._map.at("fbeg"))/module0._map.at("fstep");
	}

	// Формирование заголовка файла амплитудных измерений
	dataHeader xml_amplitudes::getAmplitudesHeader(void)
	{
		struct tm newtime = getUTC();

		// Заполнение заголовка файла.
		dataHeader header;
		header.ver = getVersion();
		header.time_sound = newtime;
		header.height_min = 0; // начальная высота сохранённых данных, м
		header.height_max = header.height_min + (getHeightCount() - 1) * getHeightStep(); // конечная высота сохранённых данных, м
		header.height_step = getHeightStep(); // шаг по высоте, м
		header.count_height = getHeightCount(); // число высот (реально измеренных)
		header.pulse_frq = getPulseFrq(); // частота зондирующих импульсов, Гц
		header.count_modules = getModulesCount(); // количество модулей зондирования

		return header;
	}
}