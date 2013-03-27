#include "CommonProtocolDecoder.h"

#include "../common/Ninja.h"

CommonProtocolDecoder::CommonProtocolDecoder()
{
	m_nPulseLength = 0;
}

boolean CommonProtocolDecoder::decode(RFPacket* pPacket)
{
	if(pPacket->getSize() != 50)
		return false;

	// Pulse length should be end-gap divided by 31
	m_nPulseLength = pPacket->get(pPacket->getSize() - 1) / 31;
	m_nCode = 0;

	// 50% tolerance, quite a lot, could/ be implemented more efficient
	word nTolerance = m_nPulseLength * 0.5;
	word nMin1 = m_nPulseLength - nTolerance;
	word nMax1 = m_nPulseLength + nTolerance;
	word nMin3 = 3*m_nPulseLength - nTolerance;
	word nMax3 = 3*m_nPulseLength + nTolerance;
	word nHighPulse = 0;

	// 24 bit => 48 pulses on/off
	for(int i = 0; i < 24; i++)
	{
		nHighPulse = pPacket->next();

		// Simply skip low pulse, for more accurate decoding this could be checked too
		pPacket->next();

		// Zero bit
		if(nHighPulse >= nMin1 && nHighPulse <= nMax1)
			;
		else if(nHighPulse >= nMin3 && nHighPulse <= nMax3)
			m_nCode += 1;
		else
		{
			m_nCode = 0;
			break;
		}
		
		m_nCode <<= 1;
	}

	m_nCode >>= 1;

	return (m_nCode != 0);
}

void CommonProtocolDecoder::fillPacket(NinjaPacket* pPacket)
{
	pPacket->setEncoding(ENCODING_COMMON);
	pPacket->setTiming(m_nPulseLength);
	pPacket->setType(TYPE_DEVICE);
	pPacket->setGuid(0);
	pPacket->setDevice(ID_ONBOARD_RF);
	pPacket->setData(m_nCode);
}