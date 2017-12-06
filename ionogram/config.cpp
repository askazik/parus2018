// ===========================================================================
// �������� � ���������������� ������ ������������.
// ===========================================================================
#include "config.h"

/// <summary>
/// �������� ������ � ����������� ��� ������ � �������������, ���������� � xml-�����.
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
		_measurement = mes;
		_fullFileName = fullName;

		// ������� ���������������� ����.
		XML::XMLError eResult = _document.LoadFile(fullName.c_str());
		if (eResult != tinyxml2::XML_SUCCESS) 
			throw std::runtime_error("������ �������� ����������������� ����� <" + fullName + ">.");

		// ������� ������������ �������.
		_xml_root = _document.RootElement();
		if(_xml_root == nullptr)
			throw std::runtime_error("� ���������������� ����� ����������� �������� ������� <parus>.");

		// ������� ��������� � ������ ���������.
		findMeasurement(mes);
		
		// �������� header
		_xml_header = _xml_measurement->FirstChildElement("header");
		_header.fill(_xml_header);
		if(_xml_header == nullptr)
		{
			std::string tmp(MeasurementNames[mes]);
			throw std::runtime_error("� ����� ��������� <" + tmp + "> ����������� ������� <header>.");
		}
		
		// �������� ���������� module
		const XML::XMLElement *module = _xml_measurement->FirstChildElement("module");
		if(module == nullptr)
		{
			std::string tmp(MeasurementNames[mes]);
			throw std::runtime_error("� ����� ��������� <" + tmp + "> ����������� ������� <module>.");
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
	/// ������������ ��������� ����� ���������.
	/// </summary>
	/// <returns>��������� ionHeaderNew2 ��� ������ � ��������� �����.</returns>
	ionHeaderNew2 xml_ionogram::getIonogramHeader(void)
	{
		ionHeaderNew2 _out;
		unit module0 = getModule(0);
		struct tm newtime = getUTC();

		// ���������� ��������� �����.
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
	/// ���������� ���������� ������ ������������ ����������.
	/// </summary>
	/// <returns></returns>
	unsigned xml_ionogram::getFrequenciesCount(void)
	{
		unit module0 = getModule(0);
		return 1 + (module0._map.at("fend") - module0._map.at("fbeg"))/module0._map.at("fstep");
	}

	// ������������ ��������� ����� ����������� ���������
	dataHeader xml_amplitudes::getAmplitudesHeader(void)
	{
		struct tm newtime = getUTC();

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
}