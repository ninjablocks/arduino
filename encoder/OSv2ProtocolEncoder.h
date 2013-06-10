#ifndef OSV2PROTOCOLENCODER_H
#define OSV2PROTOCOLENCODER_H

#include "Encoder.h"

class OSv2ProtocolEncoder :
	public Encoder
{
private:
	unsigned long	m_nCode;
	int				m_nPulseLength;
	int				m_nTriplePulseLength;

public:
	OSv2ProtocolEncoder(int nPulseLength);

	void	setCode(unsigned long long nCode);
	void	encode(RFPacket* pPacket);
};

#endif