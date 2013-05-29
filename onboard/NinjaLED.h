#ifndef NINJALED_H
#define NINJALED_H

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

class NinjaLED
{
private:
	long	m_nEyesColor;
	long long	m_nStatColor;
	long	m_nTempStatColor;
	bool 	blinkyEnabled;


public:
	NinjaLED();

	void	setup();
	void 	timerSetup();
	
	void	blinkEyes();
	void	blinkStat();
	
	long	getEyesColor();
	void	setEyesColor(long nColor);
	long	getStatColor();
	void	setStatColor(unsigned long long nColor);
	void 	statToggle();
	void	enableBlinky();
	void	disableBlinky();
	void 	enableBlinky(int period, int dutyCycle);
	void 	statOn();
	void 	statOff();

	unsigned int 	m_nPeriod;
	byte	m_nDutyCycle;
};

#endif