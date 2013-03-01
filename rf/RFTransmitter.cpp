#include "RFTransmitter.h"

RFTransmitter::RFTransmitter()
{
}

void RFTransmitter::setup(int nOutputPin)
{
	m_nOutputPin = nOutputPin;

	pinMode(m_nOutputPin, OUTPUT);		// Set pin to output
	digitalWrite(m_nOutputPin, LOW);	// Set pin low
}

void RFTransmitter::send(RFPacket* pPacket, int nRepetitions)
{
	for(int i = 0; i < nRepetitions; i++)
	{
		while(pPacket->hasNext())
		{
			// Even position high, odd low
			if((pPacket->getPosition() & 1) == 0)
				digitalWrite(m_nOutputPin, HIGH);
			else
				digitalWrite(m_nOutputPin, LOW);

			delayMicroseconds(pPacket->next());
		}

		// Rewind packet to (maybe) resend it
		pPacket->rewind();
	}
}
