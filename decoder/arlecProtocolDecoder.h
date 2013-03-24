#ifndef ARLECPROTOCOLDECODER_H
#define ARLECPROTOCOLDECODER_H

#include "Decoder.h"

class arlecProtocolDecoder :
	public Decoder
{
private:
	int 			m_nPulseLength;
	unsigned long 	m_nCode;

public:
	arlecProtocolDecoder();

	boolean decode(RFPacket* pPacket);
	void	fillPacket(NinjaPacket* pPacket);
};

#endif