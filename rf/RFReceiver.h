#ifndef RFRECEIVER_H
#define RFRECEIVER_H

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include "RFPacket.h"

class RFReceiver
{
protected:
	boolean			m_bCapture;
	boolean			m_bDataAvailable;
	RFPacket		m_PacketReceive;

private:
	static RFReceiver*	pInstance;
	
public:
	RFReceiver();

	void			start();
	void			stop();
	
	RFPacket*		getPacket();
	void			purge();
	
	void			onSignalChange();
	
	static void		onInterrupt();
};

#endif