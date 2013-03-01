#ifndef RFTRANSMITTER_H
#define RFTRANSMITTER_H

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include "RFPacket.h"

class RFTransmitter
{
protected:
	int		m_nOutputPin;

public:
	RFTransmitter();

	void	send(RFPacket* pPacket, int nRepetitions);
	void	setup(int nOutputPin);
};

#endif