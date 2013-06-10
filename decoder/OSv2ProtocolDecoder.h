#ifndef OSV2PROTOCOLDECODER_H
#define OSV2PROTOCOLDECODER_H

#include "Decoder.h"

class OSv2ProtocolDecoder :
	public Decoder
{
private:
	int 			m_nPulseLength;
	unsigned long long	m_nCode;


	void resetDecoder();

public:
	OSv2ProtocolDecoder();

	boolean decode(RFPacket* pPacket);
	void	fillPacket(NinjaPacket* pPacket);
	void reportSerial();
	void gotBit(char value);	
		void manchester(char value);
};

#endif