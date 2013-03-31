#ifndef HE330PROTOCOLENCODER_H
#define HE330PROTOCOLENCODER_H

#include "Encoder.h"

class HE330v2ProtocolEncoder :
	public Encoder
{
private:
	unsigned long	m_nCode;
	int				m_nPulseLength;
	int				m_nTriplePulseLength;
	void 			encodeZero(RFPacket* pPacket);
	void 			encodeOne(RFPacket* pPacket);	

public:
	HE330v2ProtocolEncoder(int nPulseLength);

	void	setCode(unsigned long long nCode);
	void	encode(RFPacket* pPacket);
};

#endif