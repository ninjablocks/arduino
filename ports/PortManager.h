#ifndef PORTMANAGER_H
#define PORTMANAGER_H

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include "Port.h"

#define NUM_PORTS	3

class PortManager
{
private:
	Port*	m_Ports[NUM_PORTS];

public:
	PortManager();

	void	check();
	void	handle(NinjaPacket* pPacket);
};

#endif