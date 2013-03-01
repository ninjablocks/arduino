#include "JsonSerial.h"

#define BUFFER_SIZE	128

JsonSerial jsonSerial;

JsonSerial::JsonSerial()
{
	m_Buffer = (char*) malloc(BUFFER_SIZE);
}

void JsonSerial::setup(unsigned long nBaudRate)
{
	Serial.begin(nBaudRate);
}

bool JsonSerial::read(NinjaPacket* pPacket)
{
	int i = 0;

	if(!Serial.available()) 
		return false;

	while(Serial.available() > 0)
	{
		if(i >= BUFFER_SIZE)
			break;

		m_Buffer[i] = Serial.read();
		delayMicroseconds(2000); // Set baudrate higher to be able to lower this value

		i++;
	}

	// Add end of string character
	m_Buffer[i] = 0x0;

	return pPacket->fromJson(m_Buffer);
}

void JsonSerial::printHex(byte b)
{
	if(b < 0x10)
		Serial.print("0");

	Serial.print(b, HEX);
}

