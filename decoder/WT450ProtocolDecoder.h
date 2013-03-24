#ifndef WT450PROTOCOLDECODER_H
#define WT450PROTOCOLDECODER_H

#include "Decoder.h"

class WT450ProtocolDecoder :
	public Decoder
{
private:
	int 			m_nPulseLength;
	unsigned long long	m_nCode;

public:
	WT450ProtocolDecoder();

	boolean decode(RFPacket* pPacket);
	void	fillPacket(NinjaPacket* pPacket);
};

#endif