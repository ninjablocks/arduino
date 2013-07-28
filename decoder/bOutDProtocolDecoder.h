#ifndef bOutDProtocolDecoder_H
#define bOutDProtocolDecoder_H

#include "Decoder.h"

class bOutDProtocolDecoder :
	public Decoder
{
private:
	int 			m_nPulseLength;
	unsigned long long	m_nCode;

public:
	bOutDProtocolDecoder();

	boolean decode(RFPacket* pPacket);
	void	fillPacket(NinjaPacket* pPacket);
};

#endif