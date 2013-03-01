#ifndef DECODER_H
#define DECODER_H

#include "../rf/RFPacket.h"
#include "../common/NinjaPacket.h"

class Decoder
{
public:
	virtual boolean decode(RFPacket* pPacket) = 0;
	virtual void	fillPacket(NinjaPacket* pPacket) = 0;
};

#endif