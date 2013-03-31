#include "HE330v2ProtocolDecoder.h"

#include "../common/Ninja.h"

HE330v2ProtocolDecoder::HE330v2ProtocolDecoder()
{
	m_nPulseLength = 0;
}

boolean HE330v2ProtocolDecoder::decode(RFPacket* pPacket)
{
	if(pPacket->getSize() != 132)  //RF Packet size for WT450 varies with value of the payload
		return false;

	word m_nPluseLow0 = 275;
	word m_nPulseLow1 = 1225;
	m_nCode = 0;

	// 30% tolerance is still a quite a lot, could/ be implemented more efficient
	word nTolerance = m_nPluseLow0 * 0.3;
	word nMin0 = m_nPluseLow0 - nTolerance;
	word nMax0 = m_nPluseLow0 + nTolerance;
	word nMin1 = m_nPulseLow1 - nTolerance;
	word nMax1 = m_nPulseLow1 + nTolerance;
	word nLowPulse = 0;
	
	
	//First pulse is just sync. 
	pPacket->next();
	pPacket->next();
	
	//Every bit starts with a shirt high pulse. Skipping that.
	pPacket->next();
	
	//pPacket->print();

	for(int i = 0; i < 32; i++)
	{
		nLowPulse = pPacket->next();
		//Serial.println(nLowPulse);


		// Zero bit
		if(nLowPulse >= nMin0 && nLowPulse <= nMax0)				//long pulse
			;
		else if(nLowPulse >= nMin1 && nLowPulse <= nMax1)			//short pulse
		{
			m_nCode += 1;
		}
		else
		{
			m_nCode = 0;
			break;

		}
		
		m_nCode <<= 1;
		
		//From there on, go to every 4th pulse, because every bit is repeated once to the wire
		// Data 0 = Wire 01				// Data 1 = Wire 10
		
		pPacket->next();										
		pPacket->next();
		pPacket->next();
	}

	m_nCode >>= 1;
	return (m_nCode != 0);
}

void HE330v2ProtocolDecoder::fillPacket(NinjaPacket* pPacket)
{
	pPacket->setEncoding(ENCODING_HE330);
	pPacket->setTiming(m_nPulseLength);
	pPacket->setType(TYPE_DEVICE);
	pPacket->setGuid(0);
	pPacket->setDevice(ID_ONBOARD_RF);
	pPacket->setData(m_nCode);
}