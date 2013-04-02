#include "PortManager.h"

#include "../common/Heartbeat.h"

PortManager::PortManager()
{
	m_Ports[0] = new Port(PORT1);
	m_Ports[1] = new Port(PORT2);
	m_Ports[2] = new Port(PORT3);
}

void PortManager::check()
{
	int nCurValue = 0;
	int nOldValue = 0;

	for(int i = 0; i < NUM_PORTS; i++)
	{
		// 1. Check port type
		nOldValue = m_Ports[i]->getType();
		m_Ports[i]->checkType();
		nCurValue = m_Ports[i]->getType();
	
		if(nOldValue != nCurValue)
		{
			NinjaPacket packet;
			
			packet.setGuid(i + 1);
			packet.setData(0);
		
			if(nCurValue != -1)
			{
				packet.setType(TYPE_PLUGIN);
				packet.setDevice(nCurValue);
			}
			else
			{
				packet.setType(TYPE_UNPLUG);
				packet.setDevice(nOldValue);
			}
			
			packet.printToSerial();
		}
		
		if(m_Ports[i]->isButton())
		{
			nOldValue = m_Ports[i]->getValue();
			m_Ports[i]->checkValue();
			nCurValue = m_Ports[i]->getValue();

			// Send only ON events
			if(nOldValue != nCurValue && nCurValue > 0)
			{
				NinjaPacket packet;
				
				packet.setType(TYPE_DEVICE);
				packet.setGuid(i + 1);
				packet.setDevice(m_Ports[i]->getType());
				packet.setData(nCurValue);

				packet.printToSerial();
			}
		}
		else if(heartbeat.isExpired() && m_Ports[i]->isSensor())
		{
			m_Ports[i]->checkValue();
		
			NinjaPacket packet;

			packet.setType(TYPE_DEVICE);
			packet.setGuid(i + 1);
			packet.setDevice(m_Ports[i]->getType());
			packet.setData(m_Ports[i]->getValue());

			packet.printToSerial();
		}
	}
}

void PortManager::handle(NinjaPacket* pPacket)
{
	if(pPacket->getGuid() < 1 || pPacket->getGuid() > 3)
		return;

	m_Ports[pPacket->getGuid() - 1]->handle(pPacket);

	pPacket->setType(TYPE_ACK);
	pPacket->printToSerial();
}