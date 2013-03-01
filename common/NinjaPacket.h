#ifndef NINJAPACKET_H
#define NINJAPACKET_H

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#define TYPE_DEVICE		1
#define TYPE_PLUGIN		2
#define TYPE_UNPLUG		3
#define TYPE_ACK		4
#define TYPE_ERROR		5

class NinjaPacket
{
private:
	int				m_nType;
	int				m_nGuid;
	int				m_nDevice;
	unsigned long	m_nData;

	static char*	DELIMITERS;

private:
	void			printData();
	void			printDataBinary();
	void			printDataHex();

public:
	NinjaPacket();

	int				getType();
	void			setType(int nType);
	int				getGuid();
	void			setGuid(int nGuid);
	int				getDevice();
	void			setDevice(int nDevice);
	unsigned long	getData();
	void			setData(unsigned long nData);

	bool			fromJson(char* strJson);
	void			printToSerial();
};

#endif