#include "bInDProtocolEncoder.h"

#define TOPBIT		0x80000000

bInDProtocolEncoder::bInDProtocolEncoder(int nPulseLength)
{
	m_nCode = 0;
	m_nPulseLength = nPulseLength;
	m_nTriplePulseLength = m_nPulseLength * 2;
}

void bInDProtocolEncoder::setCode(unsigned long long nCode)
{
	m_nCode = (unsigned long)nCode;
//	Serial.println(m_nCode);
}

void bInDProtocolEncoder::setCode(byte* dataPointer, byte pos)
{
	return;
}


void bInDProtocolEncoder::encode(RFPacket* pPacket)
{
	pPacket->reset();

	m_nCode <<= 8;

	// Add data
	for (int i = 0; i < 24; i++)
	{
		if(m_nCode & TOPBIT)
		{
			pPacket->append(m_nTriplePulseLength);
			pPacket->append(m_nPulseLength);
		}
		else
		{
			pPacket->append(m_nPulseLength);
			pPacket->append(m_nTriplePulseLength);
		}

		m_nCode <<= 1;
	}

	// Finish off with an extral long pulse, followed by a gap of 14 pulses
	pPacket->append(3000);
	pPacket->append(7000);
	

	pPacket->rewind();
}