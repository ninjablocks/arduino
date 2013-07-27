#ifndef BINDPROTOCOLDECODER_H
#define BINDPROTOCOLDECODER_H

#include "Decoder.h"

class bInDProtocolDecoder :
	public Decoder
{
private:
	int 			m_nPulseLength;
	unsigned long 	m_nCode;

public:
	bInDProtocolDecoder();

	boolean decode(RFPacket* pPacket);
	void	fillPacket(NinjaPacket* pPacket);
};

#endif