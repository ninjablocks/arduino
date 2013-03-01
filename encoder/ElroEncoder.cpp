#include "ElroEncoder.h"

#define NUM_BITS_HOUSECODE	5
#define TOPBIT_HOUSECODE	0x10

ElroEncoder::ElroEncoder() : CommonProtocolEncoder(325)
{
}

void ElroEncoder::set(int nHouseCode, char c, boolean bOn)
{
	unsigned long nCode = 0;

	// Encode house code
	for(int i = 0; i < NUM_BITS_HOUSECODE; i++)
	{
		if(nHouseCode & TOPBIT_HOUSECODE)
			nCode <<= 2;
		else
		{
			nCode <<= 1;
			nCode += 1;
			nCode <<= 1;
		}
		
		nHouseCode <<= 1;
	}
	
	nCode <<= 10;

	// Now the character and on/off
	switch(c)
	{
		case 'A':
			nCode += 0x55;
			break;
		case 'B':
			nCode += 0x115;
			break;
		case 'C':
			nCode += 0x145;
			break;
		case 'D':
			nCode += 0x151;
			break;
	}
	
	nCode <<= 4;

	nCode += (bOn ? 1 : 4);

	setCode(nCode);
}
