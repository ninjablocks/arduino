#ifndef ARLECPROTOCOLENCODER_H
#define ARLECPROTOCOLENCODER_H

#include "Encoder.h"

class arlecProtocolEncoder :
	public Encoder
{
private:
	unsigned long	m_nCode;
	int				m_nPulseLength;
	int				m_nTriplePulseLength;

public:
	arlecProtocolEncoder(int nPulseLength);

	void	setCode(unsigned long long nCode);
	void	encode(RFPacket* pPacket);
};

#endif