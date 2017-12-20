#ifndef __PARUSEXCEPTION_H__
#define __PARUSEXCEPTION_H__

#include <exception>
#include <ostream>

class CParusException: public std::exception
{
public:
	CParusException(void){};
	~CParusException(void){};

	virtual const char* what() const throw()
	{
		return "The Parus exception happened.";
	}
	friend std::ostream& operator<<(std::ostream &os, const CParusException& obj);
};

class CADCOverflowException: public CParusException
{
protected:
	unsigned _height_number; // номер высоты
	bool _overflowRe; // переполнение в первом отсчЄте (re) 
	bool _overflowIm; // переполнение во втором отсчЄте (im)
public:
	CADCOverflowException(): 
	  _height_number(0), _overflowRe(false), _overflowIm(false) {};
	CADCOverflowException(unsigned h_num, bool isRe, bool isIm): 
	  _height_number(h_num), _overflowRe(isRe), _overflowIm(isIm) {};
	~CADCOverflowException(void){};

	CADCOverflowException(const CADCOverflowException &obj)
    {      
        _height_number = obj.getHeightNumber();
		_overflowRe = obj.getOverflowRe();
		_overflowIm = obj.getOverflowIm();
    }

	virtual const char* what() const throw()
	{
		return "The ADC overflow exception happened.";
	}
	const unsigned getHeightNumber() const throw()
	{
		return _height_number;
	}
	const bool getOverflowRe() const throw()
	{
		return _overflowRe;
	}
	const bool getOverflowIm() const throw()
	{
		return _overflowIm;
	}
	friend std::ostream& operator<<(std::ostream &os, const CADCOverflowException& obj);
};

class CFrequencyException: public CADCOverflowException
{
	unsigned _frequency; // частота, к√ц
public:
	CFrequencyException(void): CADCOverflowException(), _frequency(0) {}
	CFrequencyException(CADCOverflowException &e, unsigned frequency): _frequency(frequency) 
	{
		_height_number = e.getHeightNumber();
		_overflowRe = e.getOverflowRe();
		_overflowIm = e.getOverflowIm();
	}
	~CFrequencyException(void){};

	virtual const char* what() const throw()
	{
		return "The ADC overflow for current frequency exception happened.";
	}
	const unsigned getFrequency() const throw()
	{
		return _frequency;
	}
	friend std::ostream& operator<<(std::ostream &os, const CFrequencyException& obj);
};

#endif // __PARUSEXCEPTION_H__