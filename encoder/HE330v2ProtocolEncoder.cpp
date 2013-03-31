#include "HE330v2ProtocolEncoder.h"

#define TOPBIT		0x80000000

HE330v2ProtocolEncoder::HE330v2ProtocolEncoder(int nPulseLength)
{
	m_nCode = 0;
	m_nPulseLength = 275;//nPulseLength;
	m_nTriplePulseLength = 1225;//m_nPulseLength * 3;
}

void HE330v2ProtocolEncoder::setCode(unsigned long long nCode)
{
	m_nCode = (unsigned long)nCode;
}

void HE330v2ProtocolEncoder::encode(RFPacket* pPacket)
{
	pPacket->reset();
	pPacket->append(m_nPulseLength);
	pPacket->append(m_nTriplePulseLength*2);
	

	// Add data
	for (int i = 0; i < 32; i++)
	{
		if(m_nCode & TOPBIT)
		{
			encodeOne(pPacket);			
		}
		else
		{
			encodeZero(pPacket);
		}

		m_nCode <<= 1;
	}

	// Finish off with one short  (high) pulse  followed by a gap of 10ms 
	pPacket->append(m_nPulseLength);
	pPacket->append(10000);

	pPacket->rewind();
}

void HE330v2ProtocolEncoder::encodeOne(RFPacket* pPacket)
{
	pPacket->append(m_nPulseLength);
	pPacket->append(m_nTriplePulseLength);
	pPacket->append(m_nPulseLength);
	pPacket->append(m_nPulseLength);
}

void HE330v2ProtocolEncoder::encodeZero(RFPacket* pPacket)
{
	pPacket->append(m_nPulseLength);
	pPacket->append(m_nPulseLength);
	pPacket->append(m_nPulseLength);
	pPacket->append(m_nTriplePulseLength);
}