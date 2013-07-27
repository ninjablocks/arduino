#ifndef OSV2PROTOCOLENCODER_H
#define OSV2PROTOCOLENCODER_H

#include "Encoder.h"

class OSv2ProtocolEncoder :
	public Encoder
{
private:
	unsigned long	m_nCode;
	int				m_nPulseLength;
	int				m_nDoublePulseLength;
	byte*			dataArray;
	byte 			arraySize;
	char 			prevBit;
	void 			manchesterEncode(char cBit, RFPacket* pPacket);
	void 			encodeBit(char val, RFPacket* pPacket);
	
public:
	OSv2ProtocolEncoder(int nPulseLength);

	void	setCode(unsigned long long nCode);
	void	encode(RFPacket* pPacket);
	void 	setCode(byte* dataPointer, byte pos);

};

#endif