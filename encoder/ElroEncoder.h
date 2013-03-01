#ifndef ELROENCODER_H
#define ELROENCODER_H

#include "CommonProtocolEncoder.h"

class ElroEncoder :
	public CommonProtocolEncoder
{
public:
	ElroEncoder();

	void	set(int nBits, char c, boolean bOn);
};

#endif