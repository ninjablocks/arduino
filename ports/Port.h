#ifndef PORT_H
#define PORT_H

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#define PORT1	1
#define PORT2	2
#define PORT3	3

#include "../common/NinjaPacket.h"

class Port
{
private:
	byte	m_nId;
	byte	m_nPinId;
	byte	m_nPinDigital;
	byte	m_nPinAnalog;
	
	int		m_nType;
	bool	m_bIsSensor;
	bool	m_bIsButton;
	int		m_nValue;
	
public:
	Port(int nId);

	void	checkType();
	int 	getType();

	void	checkValue();
	int 	getValue();
	bool	isSensor();
	bool	isButton();

	bool	handle(NinjaPacket* pPacket);
	
};

#endif