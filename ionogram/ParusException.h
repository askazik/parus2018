#ifndef __PARUSEXCEPTION_H__
#define __PARUSEXCEPTION_H__

#include <exception>

class CParusException: public std::exception
{
public:
	CParusException(void){};
	~CParusException(void){};

	virtual const char* what() const throw()
	{
		return "The Parus exception happened.";
	}
};

class CADCOverflowException: public CParusException
{
protected:
	unsigned _height; // высота, м
	bool _overflowRe; // переполнение в первом отсчЄте (re) 
	bool _overflowIm; // переполнение во втором отсчЄте (im)
public:
	CADCOverflowException(): 
	  _height(0), _overflowRe(false), _overflowIm(false) {};
	CADCOverflowException(unsigned height, bool isRe, bool isIm): 
	  _height(height), _overflowRe(isRe), _overflowIm(isIm) {};
	~CADCOverflowException(void){};

	CADCOverflowException(const CADCOverflowException &obj)
    {      
        _height = obj.getHeight();
		_overflowRe = obj.getOverflowRe();
		_overflowIm = obj.getOverflowIm();
    }

	virtual const char* what() const throw()
	{
		return "The ADC overflow exception happened.";
	}
	const unsigned getHeight() const throw()
	{
		return _height;
	}
	const bool getOverflowRe() const throw()
	{
		return _overflowRe;
	}
	const bool getOverflowIm() const throw()
	{
		return _overflowIm;
	}
};

class CFrequencyException: public CADCOverflowException
{
	unsigned _frequency; // частота, к√ц
public:
	CFrequencyException(void): CADCOverflowException(), _frequency(0) {}
	CFrequencyException(CADCOverflowException &e, unsigned frequency): _frequency(frequency) 
	{
		_height = e.getHeight();
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
};

#endif // __PARUSEXCEPTION_H__