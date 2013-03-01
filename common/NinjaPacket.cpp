#include "NinjaPacket.h"

#include "../common/Ninja.h"
#include "../serial/JsonSerial.h"

char* NinjaPacket::DELIMITERS = strdup("{}[]:,\" \r\n");

extern void printHex(byte b);

#define TOPBIT	0x800000

NinjaPacket::NinjaPacket()
{
}

int NinjaPacket::getType()
{
	return m_nType;
}

void NinjaPacket::setType(int nType)
{
	m_nType = nType;
}

int NinjaPacket::getGuid()
{
	return m_nGuid;
}

void NinjaPacket::setGuid(int nGuid)
{
	m_nGuid = nGuid;
}

int NinjaPacket::getDevice()
{
	return m_nDevice;
}

void NinjaPacket::setDevice(int nDevice)
{
	m_nDevice = nDevice;
}

unsigned long NinjaPacket::getData()
{
	return m_nData;
}

void NinjaPacket::setData(unsigned long nData)
{
	m_nData = nData;
}

bool NinjaPacket::fromJson(char* strJson)
{
	// Very slim way to decode the JSON data
	char* ptr = strtok(strJson, DELIMITERS);

	m_nGuid = -1;
	m_nDevice = 0;
	m_nData = 0;
	
	while(ptr != NULL)
	{
		if(ptr[0] == 'D')
		{
			if(ptr[1] == 'A')
			{
				char* cTmp = strtok(NULL, DELIMITERS);
			
				// Parse data
				if(strlen(cTmp) < 12)
					m_nData = strtoul(cTmp, NULL, 16); // Hex data
				else
					m_nData = strtoul(cTmp, NULL, 2); // Binary data
			}
			else if(ptr[1] == 0x0)
			{
				// Parse device id
				m_nDevice = atoi(strtok(NULL, DELIMITERS));
			}
			else if(ptr[1] == 'E')
			{
				// Assume this is a DEVICE packet
				m_nType = TYPE_DEVICE;
			}
		}
		else if (ptr[0] == 'G')
		{
			// Parse guid
			m_nGuid = atoi(strtok(NULL, DELIMITERS));
		}
		else if(ptr[0] == 'A')
		{
			// ACK => no parsing needed
			if(ptr[1] == 'C')
				break;
		}

		ptr = strtok(NULL, DELIMITERS);
	}

	return (m_nGuid != -1 && m_nDevice != 0);
}

void NinjaPacket::printToSerial()
{
	Serial.print("{\"");
	
	switch(m_nType)
	{
		case TYPE_DEVICE:
			Serial.print("DEVICE");
			break;

		case TYPE_PLUGIN:
			Serial.print("PLUGIN");
			break;

		case TYPE_UNPLUG:
			Serial.print("UNPLUG");
			break;

		case TYPE_ACK:
			Serial.print("ACK");
			break;

		case TYPE_ERROR:
			Serial.print("ERROR");
			break;
	}

	Serial.print("\":[{\"G\":\"");
	Serial.print(m_nGuid);
	Serial.print("\",\"V\":0,\"D\":");
	Serial.print(m_nDevice);
	Serial.print(",\"DA\":");

	printData();

	Serial.println("}]}");
}

void NinjaPacket::printData()
{
	if(m_nType == TYPE_DEVICE || m_nType == TYPE_ACK)
	{
		if(m_nDevice == ID_ONBOARD_RF)
			printDataBinary();
		else if(m_nDevice == ID_VERSION_NUMBER)
			Serial.print(VERSION);
		else if(m_nDevice == ID_STATUS_LED || m_nDevice == ID_NINJA_EYES)
			printDataHex();
		else
			Serial.print(m_nData);
	}
	else
		Serial.print(0);
}

void NinjaPacket::printDataBinary()
{
	Serial.print("\"");

	// Print only 24bit
	for(int i = 0; i < 24; i++)
	{
		if(m_nData & TOPBIT)
			Serial.print(1);
		else
			Serial.print(0);

		m_nData <<= 1;
	}

	Serial.print("\"");
}

void NinjaPacket::printDataHex()
{
	byte b1 = (m_nData >> 24);
	byte b2 = ((m_nData >> 16) & 0xFF);
	byte b3 = ((m_nData >> 8)& 0xFF);
	byte b4 = (m_nData & 0xFF);

	Serial.print("\"");

	//printHex(b1);
	jsonSerial.printHex(b2);
	jsonSerial.printHex(b3);
	jsonSerial.printHex(b4);

	Serial.print("\"");
}
