#include "WT450ProtocolDecoder.h"

WT450ProtocolDecoder::WT450ProtocolDecoder()
{
	m_nPulseLength = 0;
}

boolean WT450ProtocolDecoder::decode(RFPacket* pPacket)
{
	//if(pPacket->getSize() != 64)  //RF Packet size for WT450 varies with value of the payload
	//	return false;

	m_nPulseLength = 1000;
	m_nCode = 0;

	// 30% tolerance is still a quite a lot, could/ be implemented more efficient
	word nTolerance = m_nPulseLength * 0.3;
	word nMin1 = m_nPulseLength - nTolerance;
	word nMax1 = m_nPulseLength + nTolerance;
	word nMin2 = 2*m_nPulseLength - nTolerance;
	word nMax2 = 2*m_nPulseLength + nTolerance;
	word nHighPulse = 0;


	for(int i = 0; i < 36; i++)
	{
		nHighPulse = pPacket->next();


		// Zero bit
		if(nHighPulse >= nMin2 && nHighPulse <= nMax2)				//long pulse
			;
		else if(nHighPulse >= nMin1 && nHighPulse <= nMax1)			//short pulse
		{
			m_nCode += 1;
			pPacket->next();										//skip the second short pulse
		}
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

void WT450ProtocolDecoder::fillPacket(NinjaPacket* pPacket)
{
	unsigned long long header = (unsigned long long) ENCODE_WT450 << 24;
	header |= (int) m_nPulseLength;	
	pPacket->setHeader(header);
	pPacket->setType(TYPE_DEVICE);
	pPacket->setGuid(0);
	pPacket->setDevice(11);
	pPacket->setData(m_nCode);
}