#include "NinjaPacket.h"

#include "../common/Ninja.h"
#include "../serial/JsonSerial.h"

char* NinjaPacket::DELIMITERS = strdup("{}[]:,\" \r\n");

extern void printHex(byte b);

#define TOPBIT	0x800000

NinjaPacket::NinjaPacket()
{
	dataInArray = false;		//by default data is assumed to be in a long long variable m_nData
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

unsigned long long NinjaPacket::getData()
{
	return m_nData;
}

byte* NinjaPacket::getDataArray()
{
	return dataArray;
}

void NinjaPacket::setData(unsigned long long nData)
{
	m_nData = nData;
}

void NinjaPacket::setData(byte* dataPointer, byte pos)
{
	dataArray = dataPointer;
	arraySize = pos;
	dataInArray = true;
}

byte NinjaPacket::getEncoding()
{
	return m_nEncoding;
}

void NinjaPacket::setEncoding(byte nEncoding)
{
	m_nEncoding = nEncoding;
}

word NinjaPacket::getTiming()
{
	return m_nTiming;
}

void NinjaPacket::setTiming(word nTiming)
{
	m_nTiming = nTiming;
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
				char*	cTmp = strtok(NULL, DELIMITERS);
				char	nTmpVal;

				if(m_nDevice == ID_ONBOARD_RF)
				{
					// Parse header data. First byte is encoding
					cTmp[2] = 0x0;
					m_nEncoding = strtoul(&cTmp[0], NULL, 16); 

					// Byte 3 and 4 are timing. Backup value and zero-terminate string
					nTmpVal = cTmp[8];
					cTmp[8] = 0x0;
					
					// Now parse
					m_nTiming = strtoul(&cTmp[4], NULL, 16);
					
					// And restore tmp value
					cTmp[8] = nTmpVal;
					
					// Set pointer to data
					cTmp = &cTmp[8];
				}

				// Now parse data. If data is more than 32 bits, parse 2 times 32 bits
				// as there is no strtoull on Arduino
				int nDataLength = strlen(cTmp);
				int nPosLowerBytes = 0;
				
				if(nDataLength > 8)
				{
					nPosLowerBytes = (nDataLength - 8);

					nTmpVal = cTmp[nPosLowerBytes];
					cTmp[nPosLowerBytes] = 0x0;
					
					m_nData = strtoul(cTmp, NULL, 16);

					cTmp[nPosLowerBytes] = nTmpVal;

					m_nData <<= 32;
				}
				
				// Finally parse last 32bits
				m_nData += strtoul(&cTmp[nPosLowerBytes], NULL, 16);
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
		if(m_nDevice == ID_ONBOARD_RF || m_nDevice == ID_STATUS_LED || m_nDevice == ID_NINJA_EYES)
			printDataHex();
		else if(m_nDevice == ID_VERSION_NUMBER)
			Serial.print(VERSION);
		else
			Serial.print((unsigned long)m_nData);
	}
	else
		Serial.print(0);
}

void NinjaPacket::printDataHex()
{
	Serial.print("\"");

	if(m_nDevice == ID_ONBOARD_RF)
	{
		jsonSerial.printHex(m_nEncoding);
		jsonSerial.printHex(0x0); // Reserved
		jsonSerial.printHex(m_nTiming >> 8);
		jsonSerial.printHex(m_nTiming & 0xFF);
	}

	if(m_nDevice == ID_STATUS_LED || m_nDevice == ID_NINJA_EYES)
		printHex(m_nData, 3); // For RGB colors print a minimum of 3 bytes, even if zero
	else if(!dataInArray)	
		printHex(m_nData, 1);
	else 
		printHex(dataArray, arraySize);

	Serial.print("\"");
}

void NinjaPacket::printHex(unsigned long long nDataToPrint, int nNumBytesRequired)
{
	byte* 	p = ((byte*) &nDataToPrint);
	bool	bDataSent = false;

	for(int i = 7; i >= 0; i--)
	{
		if(!bDataSent && p[i] == 0 && i >= nNumBytesRequired)
			;
		else
		{
			jsonSerial.printHex(p[i]);
			bDataSent = true;
		}
	}
}

void NinjaPacket::printHex(byte* p, byte arraySize)
{
	for(int i = 0; i < arraySize ; i++)
	{
		jsonSerial.printHex(p[i]);
	}
}