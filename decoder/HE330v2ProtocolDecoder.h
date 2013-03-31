#ifndef HE330PROTOCOLDECODER_H
#define HE330PROTOCOLDECODER_H

#include "Decoder.h"

class HE330v2ProtocolDecoder :
	public Decoder
{
private:
	int 			m_nPulseLength;
	unsigned long long	m_nCode;

public:
	HE330v2ProtocolDecoder();

	boolean decode(RFPacket* pPacket);
	void	fillPacket(NinjaPacket* pPacket);
};

#endif