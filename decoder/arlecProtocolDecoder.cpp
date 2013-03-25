#include "arlecProtocolDecoder.h"

#include "../common/Ninja.h"

arlecProtocolDecoder::arlecProtocolDecoder()
{
	m_nPulseLength = 0;
}

boolean arlecProtocolDecoder::decode(RFPacket* pPacket)
{
	if(pPacket->getSize() != 26)
		return false;
	m_nPulseLength = 330;
	m_nCode = 0;

	// 50% tolerance, quite a lot, could/ be implemented more efficient
	word nTolerance = m_nPulseLength * 0.35;
	word nMin1 = m_nPulseLength - nTolerance;
	word nMax1 = m_nPulseLength + nTolerance;
	word nMin2 = 2*m_nPulseLength - nTolerance;
	word nMax2 = 2*m_nPulseLength + nTolerance;
	word nHighPulse = 0;
	
	//skip the three initial pulse changes because they don't seem to be valid data
	pPacket->next();
	pPacket->next();
	pPacket->next();

	  
	// 11 bit => 22 pulses on/off
	for(int i = 1; i < 12; i++)
	{
		nHighPulse = pPacket->next();
		// Simply skip low pulse, for more accurate decoding this could be checked too
		pPacket->next();

		// Zero bit
		if(nHighPulse >= nMin1 && nHighPulse <= nMax1)
			;
		else if(nHighPulse >= nMin2 && nHighPulse <= nMax2)
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

void arlecProtocolDecoder::fillPacket(NinjaPacket* pPacket)
{
	pPacket->setEncoding(ENCODING_ARLEC);
	pPacket->setTiming(m_nPulseLength);
	pPacket->setType(TYPE_DEVICE);
	pPacket->setGuid(0);
	pPacket->setDevice(11);
	pPacket->setData(m_nCode);
}