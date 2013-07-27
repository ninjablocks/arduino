#include "bInDProtocolDecoder.h"

#include "../common/Ninja.h"

bInDProtocolDecoder::bInDProtocolDecoder()
{
	m_nPulseLength = 0;
}

boolean bInDProtocolDecoder::decode(RFPacket* pPacket)
{
	if(pPacket->getSize() != 50)
		return false;
	//pPacket->print();
	m_nPulseLength = 500;
	m_nCode = 0;

	// 20% tolerance, quite a lot, could/ be implemented more efficient
	word nTolerance = m_nPulseLength * 0.2;
	word nMin1 = m_nPulseLength - nTolerance;
	word nMax1 = m_nPulseLength + nTolerance;
	word nMin2 = 2*m_nPulseLength - nTolerance;
	word nMax2 = 2*m_nPulseLength + nTolerance;
	word nHighPulse = 0;
	
	 //24 bits 
	for(int i = 1; i < 25; i++)  
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

void bInDProtocolDecoder::fillPacket(NinjaPacket* pPacket)
{
	pPacket->setEncoding(ENCODING_BIND);
	pPacket->setTiming(m_nPulseLength);
	pPacket->setType(TYPE_DEVICE);
	pPacket->setGuid(0);
	pPacket->setDevice(ID_ONBOARD_RF);
	pPacket->setData(m_nCode);
}