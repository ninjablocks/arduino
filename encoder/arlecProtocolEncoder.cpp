#include "arlecProtocolEncoder.h"

#define TOPBIT		0x80000000

arlecProtocolEncoder::arlecProtocolEncoder(int nPulseLength)
{
	m_nCode = 0;
	m_nPulseLength = nPulseLength;
	m_nTriplePulseLength = m_nPulseLength * 2;
}

void arlecProtocolEncoder::setCode(unsigned long long nCode)
{
	m_nCode = (unsigned long)nCode;
}

void arlecProtocolEncoder::encode(RFPacket* pPacket)
{
	pPacket->reset();

	pPacket->append(400);
	pPacket->append(600);
	pPacket->append(400);
	
	m_nCode <<= 21;

	// Add data
	for (int i = 0; i < 11; i++)
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

	// Finish off with one pulse and a zero pulse, followed by a gap of 31 pulses
	//pPacket->append(m_nPulseLength);
	pPacket->append(11640);

	pPacket->rewind();
}