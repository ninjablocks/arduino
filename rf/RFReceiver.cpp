#include "RFReceiver.h"

RFReceiver*	RFReceiver::pInstance = NULL;

RFReceiver::RFReceiver()
{
	m_bCapture = false;
	m_bDataAvailable = false;
}

void RFReceiver::start()
{
	pInstance = this;

	purge();

	attachInterrupt(0, onInterrupt, CHANGE);
}

void RFReceiver::stop()
{
	detachInterrupt(0);

	pInstance = NULL;
}

void RFReceiver::purge()
{
	m_PacketReceive.reset();

	m_bCapture = false;
	m_bDataAvailable = false;
}

RFPacket* RFReceiver::getPacket()
{
	if(!m_bDataAvailable)
		return NULL;

	return &m_PacketReceive;
}

void RFReceiver::onSignalChange()
{
	// Data available and not processed, drop new data
	if(m_bDataAvailable)
		return;

	static unsigned long nCurrentTime = 0;
	static unsigned long nDuration = 0;
	static unsigned long nLastInterrupt = 0;

	nCurrentTime = micros();
	nDuration = nCurrentTime - nLastInterrupt;
	nLastInterrupt = nCurrentTime;

	// Sanity check
	if(nDuration > 75000)
		return;
	
	// End-gap?
	if(nDuration > 3720)
	{
		if(m_bCapture)
		{
			// Data in packet buffer and possible end-gap: Try to analyze the packet
			if(m_PacketReceive.getSize() > 0)
			{
				m_PacketReceive.append(nDuration);
				m_PacketReceive.rewind();
				
				m_bDataAvailable = true;
			}
		}
		else
			m_bCapture = true; // Start capturing
	}
	else if(m_bCapture)
	{
		// If appending fails, assume garbage was received
		if(!m_PacketReceive.append(nDuration))
		{
			m_PacketReceive.reset();
			m_bCapture = false;
		}
	}
}

void RFReceiver::onInterrupt()
{
	if(pInstance != NULL)
		pInstance->onSignalChange();
}