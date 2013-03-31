#include "CommonProtocolEncoder.h"

#define TOPBIT		0x80000000

CommonProtocolEncoder::CommonProtocolEncoder(int nPulseLength)
{
	m_nCode = 0;
	m_nPulseLength = nPulseLength;
	m_nTriplePulseLength = m_nPulseLength * 3;
}

void CommonProtocolEncoder::setCode(unsigned long long nCode)
{
	m_nCode = (unsigned long)nCode;
}

void CommonProtocolEncoder::encode(RFPacket* pPacket)
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

	// Finish off with one pulse and a zero pulse, followed by a gap of 31 pulses
	pPacket->append(m_nPulseLength);
	pPacket->append(m_nPulseLength * 31);

	pPacket->rewind();
}