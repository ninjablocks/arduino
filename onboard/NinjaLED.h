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
	long	m_nStatColor;

public:
	NinjaLED();

	void	setup();
	
	void	blinkEyes();
	void	blinkStat();
	
	long	getEyesColor();
	void	setEyesColor(long nColor);
	long	getStatColor();
	void	setStatColor(long nColor);
};

#endif