// ===========================================================================
// ������������ ���� ��� ������ � ����������������� �������.
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
	// ��������� ��� ��������� ���������
	// ===========================================================================
	struct ionHeaderNew2 { 	    // === ��������� ����� ��������� ===
		unsigned ver; // ����� ������
		struct tm time_sound; // GMT ����� ��������� ����������
		unsigned height_min; // ��������� ������, �
		unsigned height_step; // ��� �� ������, �
		unsigned count_height; // ����� �����
		unsigned switch_frequency; // ������� ������������ ������ ��������� 
		unsigned freq_min; // ��������� �������, ��� (������� ������)
		unsigned freq_max; // �������� �������, ��� (���������� ������)   
		unsigned count_freq; // ����� ������ �� ���� �������
		unsigned count_modules; // ���������� ������� ������������

		// ��������� ������������� ���������.
		ionHeaderNew2(void) : 
			ver(1),
			height_min(0), // ��� �� ��������, ��� ������������ �� �����������. ���� ��������!!!
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

	struct dataHeader { 	    // === ��������� ����� ������ ===
		unsigned ver; // ����� ������
		struct tm time_sound; // GMT ����� ��������� ������������
		unsigned height_min; // ��������� ������, � (��, ��� ���� ��� ��������� �������������)
		unsigned height_max; // �������� ������, � (��, ��� ���� ��� ��������� �������������)
		unsigned height_step; // ��� �� ������, � (�������� ���, ����������� �� ������� ���)
		unsigned count_height; // ����� ����� (������ ��������� ������ ��� ��� ������������, fifo ������ ��� 4��. �.�. �� ������ 1024 �������� ��� ���� ������������ �������)
		unsigned count_modules; // ���������� �������/������ ������������
		unsigned pulse_frq; // ������� ����������� ���������, ��
	};

	// ===========================================================================
	// ���������������� ����
	// ===========================================================================
	enum  Measurement { MEASUREMENT, IONOGRAM, AMPLITUDES}; // ������������ ��� ��������� ���������� ������������

	#pragma pack(pop)

	// ���� ������������� ������ (header, module, ...)
	struct unit
	{
		std::string _name;
		Measurement _meas;
		std::map<std::string, unsigned> _map; // ��������� ���� ����/��������
		void fill(const XML::XMLElement *parent);
	};

	// ������� ���� ����������������� xml-�����
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

		// ���������� �� ���������.
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

	// ���� ������������ ������������
	class xml_project : public xml_unit 
	{
	public:
		xml_project(std::string fullName = XML_CONFIG_DEFAULT_FILE_NAME) : xml_unit(MEASUREMENT, fullName){}
	};

	// ���� ����������
	class xml_ionogram : public xml_unit 
	{
	public:
		xml_ionogram(std::string fullName = XML_CONFIG_DEFAULT_FILE_NAME) : xml_unit(IONOGRAM, fullName){}

		// ������������ ��������� �����
		const ionHeaderNew2 getIonogramHeader();
		virtual const unsigned getFrequenciesCount();
	};

	// ���� ��������
	class xml_amplitudes : public xml_unit 
	{
	public:
		xml_amplitudes(std::string fullName = XML_CONFIG_DEFAULT_FILE_NAME) : xml_unit(AMPLITUDES, fullName){}
		const unsigned getAmplitudesFrq(unsigned i){return (getModulesCount()) ? getModule(i)._map.at("frequency") : 0;}

		// ������������ ��������� �����
		const dataHeader getAmplitudesHeader();
	};
}; // end namespace parus

#endif // __CONFIG_H__