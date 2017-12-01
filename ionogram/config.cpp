// ===========================================================================
// �������� � ���������������� ������ ������������.
// ===========================================================================
#include "config.h"

/// <summary>
/// �������� ������ � ����������� ��� ������ � ������������� ���������� � ������������.
/// </summary>
namespace parus {

	// ===========================================================================
	// ���������������� ���� XML
	// ===========================================================================
	const char* xml_unit::MeasurementNames[] = {"measurement", "ionogram", "amplitudes" }; // ����� ������ ���������

	/// <summary>
	/// Initializes a new instance of the base <see cref="xmlunit"/> class.
	/// </summary>
	/// <param name="fullName">The full file name.</param>
	/// <param name="mes">The measurement type.</param>
	xml_unit::xml_unit(Measurement mes, std::string fullName)
	{
		// ������� ���������������� ����.
		XML::XMLError eResult = _document.LoadFile(fullName.c_str());
		if (eResult != tinyxml2::XML_SUCCESS) 
			throw std::runtime_error("������ �������� ����������������� ����� <" + fullName + ">.");

		// ������� ������������ �������.
		_root = _document.RootElement();
		if(_root == nullptr)
			throw std::runtime_error("� ���������������� ����� ����������� �������� ������� <parus>.");

		// ������� ��������� � ������ ���������.
		findMeasurement(mes);
		_header = _measurement->FirstChildElement("header");
		if(_header == nullptr)
		{
			std::string tmp(MeasurementNames[mes]);
			throw std::runtime_error("� ����� ��������� <" + tmp + "> ����������� ������� <header>.");
		}
		const XML::XMLElement *module = _measurement->FirstChildElement("module");
		while (module != nullptr)
		{
			_modules.push_back(module);
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
         _measurement = _root->FirstChildElement("Measurement");
         _measurement;
         _measurement = _measurement->NextSiblingElement()
        ) 
		{
			if(!strcmp(_measurement->Attribute("name"), MeasurementNames[mes]))
				break;
		}
	}

	void xml_project::fillMeasurementModules(void)
	{
		for(auto it = _modules.begin(); it != _modules.end(); ++it) {
			MeasurementStruct out;
			const XML::XMLElement *tmp = *it;

			out.measurement = tmp->Attribute("name");
			const XML::XMLElement *dt = tmp->FirstChildElement("dt");
			out.dt = (dt == nullptr) ? 0 : dt->IntText(0);
			_queue.push_back(out);
		}
	}

	void xml_ionogram::fillMeasurementModules(void)
	{
		for(auto it = _modules.begin(); it != _modules.end(); ++it) {
			MeasurementModule out;
			const XML::XMLElement *tmp = *it;

			const XML::XMLElement *element;
			for (
				element = tmp->FirstChildElement("Measurement");
				element;
				element = element->NextSiblingElement()
			)
				{
					std::string name = element->Name();
					unsigned value = (element == nullptr) ? 0 : element->IntText(0);
					out[name.c_str()] = value;
				}

			_queue.push_back(out);
		}
	}

	/// <summary>
	/// ������������� ������ ������� ������ <see cref="xmlconfig"/>.
	/// </summary>
	/// <param name="fullName">��� ����������������� ����� xml. �� ���������: #define XML_CONFIG_DEFAULT_FILE_NAME.</param>
	/// <param name="mes">��� ������������ (����������/���������������). �� ���������: IONOGRAM.</param>
	xmlconfig::xmlconfig(std::string fullName, Measurement mes)
	{
		_fullFileName = fullName;
		_mes = mes;

		// ������� ����������.
		const XML::XMLElement *parent, *xml_mes;
		_document.LoadFile(_fullFileName.c_str());
 
		// ������� �������� name ��� ������ ���������
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

		// ��������� ������� � ������� ����������
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
 
		// ��������� �������� ������������ ����������.
		loadMeasurementHeader(xml_mes);
		// ��������� �������� ������������ ������������.
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
	/// ���������� ���������� � ���������� ����������. ���������� � ��������� ����� ������������.
	/// </summary>
	/// <param name="xml_mes">��������� �� XML �������, ���������� ���������� � ���������� ����������/������������.</param>
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
	/// �������� ���������� � ������������ ��������� ���������.
	/// </summary>
	/// <param name="xml_mes">��������� �� XML �������, ���������� ���������� � ���������� ����������/������������.</param>
	void xmlconfig::loadIonogramConfig(const XML::XMLElement *xml_mes)
	{
		const XML::XMLElement *xml_module, *xml_element;
		int value = 0;

		// �������, ��� ������ ���������� ������������
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

	// ������������ ��������� ����� ���������
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

		// ���������� ��������� �����.
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

	// ������������ ��������� ����� ����������� ���������
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

		// ���������� ��������� �����.
		dataHeader header;
		header.ver = getVersion();
		header.time_sound = newtime;
		header.height_min = 0; // ��������� ������ ���������� ������, �
		header.height_max = header.height_min + (getHeightCount() - 1) * getHeightStep(); // �������� ������ ���������� ������, �
		header.height_step = getHeightStep(); // ��� �� ������, �
		header.count_height = getHeightCount(); // ����� ����� (������� ����������)
		header.pulse_frq = getPulseFrq(); // ������� ����������� ���������, ��
		header.count_modules = getModulesCount(); // ���������� ������� ������������

		return header;
	}

	/// <summary>
	/// �������� ���������� � ������������ ����������� ���������.
	/// </summary>
	/// <param name="xml_mes">��������� �� XML �������, ���������� ���������� � ���������� ����������/������������.</param>
	void xmlconfig::loadAmplitudesConfig(const XML::XMLElement *xml_mes)
	{
		const XML::XMLElement *xml_module, *xml_element;
		int value = 0;
		int i = 0;

		// ��������� ������ ������� ����������
		_amplitudes.resize(getModulesCount());
		// ������� �� �������
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