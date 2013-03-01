#ifndef COMMONPROTOCOLENCODER_H
#define COMMONPROTOCOLENCODER_H

#include "Encoder.h"

class CommonProtocolEncoder :
	public Encoder
{
private:
	unsigned long	m_nCode;
	int				m_nPulseLength;
	int				m_nTriplePulseLength;

public:
	CommonProtocolEncoder(int nPulseLength);

	void	setCode(unsigned long nCode);
	void	encode(RFPacket* pPacket);
};

#endif