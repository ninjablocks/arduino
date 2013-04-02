#include "Port.h"

#include "../common/Ninja.h"

Port::Port(int nId)
{
	m_nId = nId;
	m_nType = 0;
	m_nValue = -1;

	if(m_nId == PORT1)
	{
		m_nPinId = 17;		// A3
		m_nPinAnalog = 14;	// A0
		m_nPinDigital = 3;
	}
	else if(m_nId == PORT2)
	{
		m_nPinId = 20;		// A6
		m_nPinAnalog = 15;	// A1
		m_nPinDigital = 5;
	}
	else if(m_nId == PORT3)
	{
		m_nPinId = 21;		// A7
		m_nPinAnalog = 16;	// A2
		m_nPinDigital = 6;
	}

	pinMode(m_nPinId, INPUT);
}

int Port::getType()
{
	return m_nType;
}

void Port::checkType()
{
	int val = analogRead(m_nPinId);

	m_bIsSensor = false;
	m_bIsButton = false;

	if (val > 178 && val < 198) // Distance (ADC Value=188)
	{
		m_nType = ID_DISTANCE_SENSOR; 
		m_bIsSensor = true;
	}
	else if (val > 250 && val < 270) // PIR (ADC Value=260 )
	{
		m_nType = ID_PIR_SENSOR;
		m_bIsButton = true;
	}
	else if (val > 365 && val < 385) // Light (ADC Value=375)
	{
		m_nType = ID_LIGHT_SENSOR;
		m_bIsSensor = true;
	}
	else if (val > 511 && val < 531) // Humidity & Temp (ADC Value=521)
	{
		m_nType = ID_HUMIDITY_TEMP_SENSOR;
		m_bIsSensor = true;
	}
	else if (val > 559 && val < 579) // Sound Sensor (ADC Value=569)
	{
		m_nType = ID_SOUND_SENSOR;
		m_bIsSensor = true;
	}
	else if (val > 654 && val < 674) // Relay Breakoutboard (ADC Value=664)
		m_nType = ID_RELAY_BOARD;
	else if (val > 782 && val < 802) // Button (ADC value=792)
	{
		m_bIsButton = true;
		m_nType = ID_BUTTON;
	}
	else
		m_nType = -1;
}

bool Port::isSensor()
{
	return m_bIsSensor;
}

bool Port::isButton()
{
	return m_bIsButton;
}

bool Port::handle(NinjaPacket* pPacket)
{
	if(pPacket->getDevice() != m_nType)
		return false;

	if(m_nType == ID_RELAY_BOARD)
	{
		pinMode(m_nPinAnalog, OUTPUT);
		digitalWrite(m_nPinAnalog, pPacket->getData());
	}

	return true;
}

int Port::getValue()
{
	return m_nValue;
}

void Port::checkValue()
{
	switch(m_nType)
	{
		case ID_BUTTON:
			pinMode(m_nPinAnalog, INPUT);
			m_nValue = digitalRead(m_nPinAnalog);
			break;
		
		case ID_LIGHT_SENSOR:
			pinMode(m_nPinAnalog, INPUT);
			m_nValue = analogRead(m_nPinAnalog);
			break;

		case ID_PIR_SENSOR:
			pinMode(m_nPinAnalog, INPUT);
			m_nValue = digitalRead(m_nPinAnalog);
			break;

		case ID_DISTANCE_SENSOR:
			pinMode(m_nPinAnalog, INPUT);
			m_nValue = analogRead(m_nPinAnalog);
			break;
		
		case ID_SOUND_SENSOR:
			pinMode(m_nPinAnalog, INPUT);
			m_nValue = analogRead(m_nPinAnalog);
			break;

		default: // Invalid sensor ID
			m_nValue = -1;
	}
}