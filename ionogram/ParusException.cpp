#include "ParusException.h"

namespace parus {

	std::ostream& operator<<(std::ostream &os, const CParusException& obj)
	{
		os << "���������\t: " << obj.what();
		return os;
	}

	std::ostream& operator<<(std::ostream &os, const CADCOverflowException& obj)
	{
		os << dynamic_cast<const CParusException &>(obj) << std::endl;
		os << "����� ������\t: " << obj.getHeightNumber() << std::endl;
		os << "������������\t: Re = " << std::boolalpha << obj.getOverflowRe() << ", Im = " << obj.getOverflowIm();
		return os;
	}

	std::ostream& operator<<(std::ostream &os, const CFrequencyException& obj)
	{
		os << dynamic_cast<const CADCOverflowException &>(obj) << std::endl;
		os << "�������, ���\t: " << obj.getFrequency();
		return os;
	}

}