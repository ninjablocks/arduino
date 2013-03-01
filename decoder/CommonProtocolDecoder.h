#ifndef COMMONPROTOCOLDECODER_H
#define COMMONPROTOCOLDECODER_H

#include "Decoder.h"

class CommonProtocolDecoder :
	public Decoder
{
private:
	int 			m_nPulseLength;
	unsigned long	m_nCode;

public:
	CommonProtocolDecoder();

	boolean decode(RFPacket* pPacket);
	void	fillPacket(NinjaPacket* pPacket);
};

#endif