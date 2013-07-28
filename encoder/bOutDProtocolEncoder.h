#ifndef bOutDProtocolEncoder_H
#define bOutDProtocolEncoder_H

#include "Encoder.h"

class bOutDProtocolEncoder :
	public Encoder
{
private:
	unsigned long long	m_nCode;
	int				m_nPulseLength;
	int				m_nTriplePulseLength;

public:
	bOutDProtocolEncoder(int nPulseLength);

	void	setCode(unsigned long long nCode);
	void	encode(RFPacket* pPacket);
	void 	setCode(byte* dataPointer, byte pos);

};

#endif