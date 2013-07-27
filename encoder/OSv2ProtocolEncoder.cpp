#include "OSv2ProtocolEncoder.h"

#define BOTBIT		0x01

/*********************************************\
Manchester encoding (as used for the Oregon Scientific v2 devices
assume the very first bit is zero.
two short pulses -> current bit is the same as previous bit
a long pulse -> current bit is ~previous bit (flips the bit)
a long pulse cannot follow one short pulse
each bit writen to the wire twice, first time negated.
Packet starts with 16 bit premble of all ones 	- ie:31 long pulses 

OSv2 uses 17 nibbles in total for this device

/*******************DINUKA********************/

OSv2ProtocolEncoder::OSv2ProtocolEncoder(int nPulseLength)
{
	m_nCode = 0;
	m_nPulseLength = 400;//nPulseLength;
	m_nDoublePulseLength = m_nPulseLength * 2;
	prevBit = 0;
}

void OSv2ProtocolEncoder::setCode(unsigned long long nCode)
{
}

void OSv2ProtocolEncoder::setCode(byte* dataPointer, byte pos)
{
	dataArray = dataPointer;
	arraySize = pos;
}

void OSv2ProtocolEncoder::encode(RFPacket* pPacket)
{
	pPacket->reset();

	byte curVal;

	//preamble - 31 ones
	for (int i = 1; i < 32; i++)
	{
		pPacket->append(m_nDoublePulseLength);
	}
	//end preamble
	
	// Add data
	for (int j = 0; j < arraySize; j++)
	{ 	
		if (j==arraySize-1)		//from the last byte, only the most significant nibble should be sent
			curVal = dataArray[j] >> 4;
		else 
			curVal = dataArray[j];
			
		for (int i = 0; i < 8; i++)
		{
			if ((j==arraySize-1) && (i==4))							//from the last byte, only the most significant nibble should be sent
				break;
				
			if(curVal & BOTBIT)									//start from the LSB and move toward MSB
			{
				encodeBit(1, pPacket);
			}
			else
			{
				encodeBit(0, pPacket);
			}
	
			curVal >>= 1;
		}
	}

	//if even number of pulses, add an extra shot pulse to return to zero 
	if(((pPacket->getSize()) & 1) == 0)			
		pPacket->append(m_nPulseLength);
		
	pPacket->append(m_nPulseLength * 16);
	//pPacket->print();
	pPacket->rewind();

}

void OSv2ProtocolEncoder::encodeBit(char val, RFPacket* pPacket)
{
		manchesterEncode(val, pPacket);
		manchesterEncode(~val, pPacket);
}

void OSv2ProtocolEncoder::manchesterEncode(char cBit, RFPacket* pPacket)
{
	if(prevBit ^ (cBit & BOTBIT)) //if last bit of cBit and preBit are NOT the same
		pPacket->append(m_nDoublePulseLength);		//to flip the bit use a long pulse
	else
	{
		pPacket->append(m_nPulseLength);			//to keep the bit the same, use two short pulse
		pPacket->append(m_nPulseLength);	
	}
	
	prevBit = (cBit & BOTBIT);

}
//{"DEVICE":[{"G":"0","V":0,"D":11,"DA":"05000204EA4C108968277054D2"}]}