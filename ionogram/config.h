// ===========================================================================
// ������������ ���� ��� ������ � ����������������� �������.
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
	// ������ ��� ����������� ���������
	// ===========================================================================
	struct myModule {  
		unsigned frq;   // ������� ������, ���
	};

	// ===========================================================================
	// ���������������� ����
	// ===========================================================================
	enum  Measurement { MEASUREMENT, IONOGRAM, AMPLITUDES}; // ������������ ��� ��������� ���������� ������������

	// ����� ��������� ��� ���������������� ����������.
	struct ionosounder { 
		unsigned ver; // ����� ������ ��� ���������� ����� �����������
		unsigned height_step; // ��� �� ������
		unsigned height_count; // ���������� �����
		unsigned pulse_count; // ��������� ������������ �� ������ �������
		unsigned attenuation; // ���������� (����������) 1/0 = ���/����
		unsigned gain;	// ��������, �� (g = value/6, 6�� = ���������� � 4 ���� �� ��������)
		unsigned pulse_frq; // ������� ����������� ���������, ��
		unsigned pulse_duration; // ������������ ����������� ���������, ���
		unsigned switch_frequency; // ������� ������������ ������, ��� (�� ������������)
		unsigned modules_count; // ����� �������
	};

	struct ionogramSettings {  
		unsigned fstep;  // ��� �� ������� ����������, ���
		unsigned fbeg;   // ��������� ������� ������, ���
		unsigned fend;   // �������� ������� ������, ���
		unsigned count;  // ���������� ������ ������������
	};

	#pragma pack(pop)

	// ����� ���� ����������������� xml-�����
	class xmlunit {
		XML::XMLDocument _document;
		Measurement _mes;

	public:
		virtual xmlunit(std::string fullName = XML_CONFIG_DEFAULT_FILE_NAME, Measurement mes = MEASUREMENT) = 0;

	};

	class xmlconfig {

	protected:
		ionosounder _device;
		std::string _fullFileName;
		Measurement _mes;
		XML::XMLDocument _document;

		// ��������� ��� ��������� ���������
		ionogramSettings _ionogram;
		// ������ �������� ��� ����������� ���������
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

		// ������������ ��������� �����
		ionHeaderNew2 getIonogramHeader(void);
		dataHeader getAmplitudesHeader(void);
	};


	//// ===========================================================================
	//// ���������
	//// ===========================================================================
	//struct myModule {  
	//	unsigned frq;   // ������� ������, ���
	//};

	//// ����� ������� � ����������������� ����� ������������.
	//class parusConfig  : public config {
 //   
	//	unsigned _ver; // ����� ������
	//	unsigned _height_step; // ��� �� ������, �
	//	unsigned _height_count; // ���������� �����
	//	unsigned _modules_count; // ���������� �������/������ ������������
	//	unsigned _pulse_count; // ��������� ������������ �� ������ �������
	//	unsigned _attenuation; // ���������� (����������) 1/0 = ���/����
	//	unsigned _gain;	// �������� (g = value/6, 6�� = ���������� � 4 ���� �� ��������)
	//	unsigned _pulse_frq; // ������� ����������� ���������, ��
	//	unsigned _pulse_duration; // ������������ ����������� ���������, ���
	//	unsigned _height_min; // ��������� ������, �� (��, ��� ���� ��� ��������� �������������)
	//	unsigned _height_max; // �������� ������, �� (��, ��� ���� ��� ��������� �������������)

	//	myModule	*_ptModules; // ��������� �� ������ �������

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

	//	unsigned getAttenuation(void){return _attenuation;} // ���������� (����������) 1/0 = ���/����
	//	unsigned getGain(void){return _gain;}	// �������� (g = value/6, 6�� = ���������� � 4 ���� �� ��������)
	//	unsigned getPulse_frq(void){return _pulse_frq;} // ������� ����������� ���������, ��
	//	unsigned getPulse_duration(void){return _pulse_duration;} // ������������ ����������� ���������, ���

	//	unsigned getHeightMin(void){return _height_min;} // ��������� ������, �� (��, ��� ���� ��� ��������� �������������)
	//	unsigned getHeightMax(void){return _height_max;} // �������� ������, �� (��, ��� ���� ��� ��������� �������������)
	//};

}; // end namespace parus

#endif // __CONFIG_H__