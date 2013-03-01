#ifndef HEARTBEAT_H
#define HEARTBEAT_H

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

class Heartbeat
{
private:
	unsigned long	m_nTimeOut;
	unsigned long	m_nCurrent;

public:
	Heartbeat();

	void	check();
	void	resume();

	bool	isExpired();
};

extern Heartbeat heartbeat;

#endif