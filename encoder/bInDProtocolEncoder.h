#ifndef bInDProtocolEncoder_H
#define bInDProtocolEncoder_H

#include "Encoder.h"

class bInDProtocolEncoder :
	public Encoder
{
private:
	unsigned long	m_nCode;
	int				m_nPulseLength;
	int				m_nTriplePulseLength;

public:
	bInDProtocolEncoder(int nPulseLength);

	void	setCode(unsigned long long nCode);
	void	encode(RFPacket* pPacket);
	void 	setCode(byte* dataPointer, byte pos);

};

#endif