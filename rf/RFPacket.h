#ifndef RFPACKET_H
#define RFPACKET_H

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#define MAX_BUFFER_SIZE	140
#define ENCODE_MAG_SW 0x01
#define ENCODE_WT450  0x02
#define ENCODE_ARLEC  0x03

class RFPacket
{
protected:
	word			m_Buffer[MAX_BUFFER_SIZE];
	int				m_nSize;
	int				m_nPosition;

public:
	RFPacket();

	boolean			append(word nValue);
	word			get(int nIndex);
	int				getPosition();
	int				getSize();
	boolean			hasNext();
	word			next();
	void			reset();
	void			rewind();
	void			print();
	void			set(int nIndex, int nValue);
};

#endif