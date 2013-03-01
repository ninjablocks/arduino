#include "RFPacket.h"

RFPacket::RFPacket()
{
	reset();
}

boolean RFPacket::append(word nValue)
{
	if(m_nPosition + 1 >= MAX_BUFFER_SIZE)
		return false;

	m_Buffer[m_nPosition] = nValue;
	
	m_nPosition++;
	m_nSize++;

	return true;
}

int RFPacket::getPosition()
{
	return m_nPosition;
}

int RFPacket::getSize()
{
	return m_nSize;
}

void RFPacket::reset()
{
	m_nPosition = 0;
	m_nSize = 0;
}

void RFPacket::rewind()
{
	m_nPosition = 0;
}

boolean RFPacket::hasNext()
{
	return (m_nPosition < m_nSize);
}

word RFPacket::next()
{
	return m_Buffer[m_nPosition++];
}

word RFPacket::get(int nIndex)
{
	return m_Buffer[nIndex];
}

void RFPacket::set(int nIndex, int nValue)
{
	m_Buffer[nIndex] = nValue;
}

void RFPacket::print()
{
	Serial.print("Size: ");
	Serial.println(m_nSize);
	Serial.print("Next: ");
	Serial.println(m_nPosition);

	/*if(m_nSize != 50)
		return;*/

	for(int i = 0; i < m_nSize; i++)
	{
		Serial.print(i);
		Serial.print(": ");
		Serial.println(m_Buffer[i]);
	}
}