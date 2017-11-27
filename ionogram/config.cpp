// ===========================================================================
// Операции с конфигурационным файлом зондирования.
// ===========================================================================
#include "config.h"

/// <summary>
/// Содержит классы и определения для работы с конфигурацией аппаратуры и эксперимента.
/// </summary>
namespace parus {

	// ===========================================================================
	// Конфигурационный файл XML
	// ===========================================================================

	/// <summary>
	/// Инициализация нового объекта класса <see cref="xmlconfig"/>.
	/// </summary>
	/// <param name="fullName">Имя конфигурационного файла xml. По умолчанию: #define XML_CONFIG_DEFAULT_FILE_NAME.</param>
	/// <param name="mes">Вид эксперимента (ионограмма/амплитудограмма). По умолчанию: IONOGRAM.</param>
	xmlconfig::xmlconfig(std::string fullName, Measurement mes)
	{
		_fullFileName = fullName;
		_mes = mes;

		// Считаем информацию.
		const XML::XMLElement *parent, *xml_mes;
		_document.LoadFile(_fullFileName.c_str());
 
		// Искомый параметр name для выбора измерения
		std::string mes_name;
		switch(mes)
		{
		case IONOGRAM:
			mes_name = "ionogram";
			break;
		case AMPLITUDES:
			mes_name = "amplitudes";
			break;
		}

		// Определим элемент с искомым параметром
		parent = _document.FirstChildElement();
		for (
         xml_mes = parent->FirstChildElement("Measurement");
         xml_mes;
         xml_mes = xml_mes->NextSiblingElement()
        ) 
		{
			if(!strcmp(xml_mes->Attribute("name"), mes_name.c_str()))
				break;
		}
 
		// Считываем желаемую конфигурацию аппаратуры.
		loadMeasurementHeader(xml_mes);
		// Считываем желаемую конфигурацию эксперимента.
		switch(mes)
		{
		case IONOGRAM:
			loadIonogramConfig(xml_mes);
			break;
		case AMPLITUDES:
			loadAmplitudesConfig(xml_mes);
			break;
		}
	}

	/// <summary>
	/// Считывание информации о настройках аппаратуры. Содержится в заголовке блока эксперимента.
	/// </summary>
	/// <param name="xml_mes">Указатель на XML элемент, сожержащий информацию о настройках аппаратуры/эксперимента.</param>
	void xmlconfig::loadMeasurementHeader(const XML::XMLElement *xml_mes)
	{
		const XML::XMLElement *xml_header, *xml_element;
		int value = 0;

		xml_header = xml_mes->FirstChildElement("header");
		xml_element = xml_header->FirstChildElement("version");
			xml_element->QueryIntText(&value);
				_device.ver = value;
		xml_element = xml_header->FirstChildElement("height_step");
			xml_element->QueryIntText(&value);
				_device.height_step = value;
		xml_element = xml_header->FirstChildElement("height_count");
			xml_element->QueryIntText(&value);
				_device.height_count = value;
		xml_element = xml_header->FirstChildElement("pulse_count");
			xml_element->QueryIntText(&value);
				_device.pulse_count = value;
		xml_element = xml_header->FirstChildElement("attenuation");
			xml_element->QueryIntText(&value);
				_device.attenuation = value;
		xml_element = xml_header->FirstChildElement("gain");
			xml_element->QueryIntText(&value);
				_device.gain = value;
		xml_element = xml_header->FirstChildElement("pulse_frq");
			xml_element->QueryIntText(&value);
				_device.pulse_frq = value;
		xml_element = xml_header->FirstChildElement("pulse_duration");
			xml_element->QueryIntText(&value);
				_device.pulse_duration = value;
		xml_element = xml_header->FirstChildElement("switch_frequency");
			xml_element->QueryIntText(&value);
				_device.switch_frequency = value;
		xml_element = xml_header->FirstChildElement("modules_count");
			xml_element->QueryIntText(&value);
				_device.modules_count = value;
	}

	/// <summary>
	/// Загрузка информации о конфигурации измерения ионограмм.
	/// </summary>
	/// <param name="xml_mes">Указатель на XML элемент, сожержащий информацию о настройках аппаратуры/эксперимента.</param>
	void xmlconfig::loadIonogramConfig(const XML::XMLElement *xml_mes)
	{
		const XML::XMLElement *xml_module, *xml_element;
		int value = 0;

		// Считаем, что модуль ионограммы единственный
		xml_module = xml_mes->FirstChildElement("module");
		xml_element = xml_module->FirstChildElement("fbeg");
			xml_element->QueryIntText(&value);
				_ionogram.fbeg = value;
		xml_element = xml_module->FirstChildElement("fend");
			xml_element->QueryIntText(&value);
				_ionogram.fend = value;
		xml_element = xml_module->FirstChildElement("fstep");
			xml_element->QueryIntText(&value);
				_ionogram.fstep = value;

		_ionogram.count = 1 + (_ionogram.fend - _ionogram.fbeg) / _ionogram.fstep;
	}

	// Формирование заголовка файла ионограмм
	ionHeaderNew2 xmlconfig::getIonogramHeader(void)
	{
		ionHeaderNew2 _out;

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

		// Заполнение заголовка файла.
		_out.time_sound = newtime;
		_out.count_freq = 1 + (_ionogram.fend - _ionogram.fbeg)/_ionogram.fstep;
		_out.count_height = getHeightCount();
		_out.count_modules = getModulesCount();
		_out.freq_max = _ionogram.fend;
		_out.freq_min = _ionogram.fbeg;
		_out.height_min = 0;
		_out.height_step = getHeightStep();
		_out.switch_frequency = _device.switch_frequency;
		_out.ver = getVersion();

		return _out;
	}

	// Формирование заголовка файла амплитудных измерений
	dataHeader xmlconfig::getAmplitudesHeader(void)
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

	/// <summary>
	/// Загрузка информации о конфигурации амплитудных измерений.
	/// </summary>
	/// <param name="xml_mes">Указатель на XML элемент, сожержащий информацию о настройках аппаратуры/эксперимента.</param>
	void xmlconfig::loadAmplitudesConfig(const XML::XMLElement *xml_mes)
	{
		const XML::XMLElement *xml_module, *xml_element;
		int value = 0;
		int i = 0;

		// Определим размер вектора параметров
		_amplitudes.resize(getModulesCount());
		// Перебор по модулям
		for (
         xml_module = xml_mes->FirstChildElement("module");
         xml_module;
         xml_module = xml_module->NextSiblingElement()
        ) 
		{
			xml_element = xml_module->FirstChildElement("frequency");
			xml_element->QueryIntText(&value);
				_amplitudes[i].frq = value;
			i++;
		}
	}
}