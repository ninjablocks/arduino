#ifndef RFPACKET_H
#define RFPACKET_H

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#define MAX_BUFFER_SIZE	256

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