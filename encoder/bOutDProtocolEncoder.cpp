#include "bOutDProtocolEncoder.h"

#define TOPBIT		0x8000000000000000LL

bOutDProtocolEncoder::bOutDProtocolEncoder(int nPulseLength)
{
	m_nCode = 0;
	m_nPulseLength = nPulseLength;
	m_nTriplePulseLength = m_nPulseLength * 2;
}

void bOutDProtocolEncoder::setCode(unsigned long long nCode)
{
	m_nCode = nCode;
//	Serial.println(m_nCode);
}

void bOutDProtocolEncoder::setCode(byte* dataPointer, byte pos)
{
	return;
}


void bOutDProtocolEncoder::encode(RFPacket* pPacket)
{
	pPacket->reset();

	m_nCode <<= 16;

	// Add data
	for (int i = 0; i < 47; i++) //encode only the first 47 bits because the 48th is only half a bit (which is taken care of by the last long pulse)
	{
		if(m_nCode & TOPBIT)
		{
			pPacket->append(m_nTriplePulseLength);
			pPacket->append(m_nPulseLength);
		}
		else
		{
			pPacket->append(m_nPulseLength);
			pPacket->append(m_nPulseLength);
		}

		m_nCode <<= 1;
	}

	// Finish off with a long pulse and a gap of 30 pulses
	pPacket->append(m_nTriplePulseLength);
	pPacket->append(9000);
	

	pPacket->rewind();
//	pPacket->print();
}