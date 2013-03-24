#include "OnBoardManager.h"

#include "../common/Ninja.h"
#include "../common/Heartbeat.h"

#include "NinjaLED.h"

#include "../decoder/CommonProtocolDecoder.h"
#include "../encoder/CommonProtocolEncoder.h"


extern NinjaLED leds;

OnBoardManager::OnBoardManager()
{
	m_nLastDecode = -1;

	m_Decoders[0] = new CommonProtocolDecoder();
}

void OnBoardManager::setup()
{
	m_Transmitter.setup(TRANSMIT_PIN);

	m_Receiver.start();
}

void OnBoardManager::check()
{
	RFPacket*	pReceivedPacket = m_Receiver.getPacket();

	// Check for unhandled RF data first
	if(pReceivedPacket != NULL)
	{
		bool bDecodeSuccessful = false;

		m_nLastDecode = -1;

		for(int i = 0; i < NUM_DECODERS; i++)
		{
			if(m_Decoders[i]->decode(pReceivedPacket))
			{
				m_nLastDecode = i;

				bDecodeSuccessful = true;
			}
		}

		if(bDecodeSuccessful)
		{
			// Blink stat LED to show activity
			leds.blinkStat();

			NinjaPacket packet;
			
			m_Decoders[m_nLastDecode]->fillPacket(&packet);
			
			packet.printToSerial();
		}

		// Purge 
		m_Receiver.purge();
	}

	// Check if heartbeat expired
	if(heartbeat.isExpired())
	{
		NinjaPacket packet;

		packet.setType(TYPE_DEVICE);
		packet.setGuid(0);
		packet.setDevice(ID_STATUS_LED);
		packet.setData(leds.getStatColor());

		packet.printToSerial();
		
		packet.setDevice(ID_NINJA_EYES);
		packet.setData(leds.getEyesColor());

		packet.printToSerial();
	}
}

void OnBoardManager::handle(NinjaPacket* pPacket)
{
	if(pPacket->getGuid() != 0)
		return;

	if(pPacket->getDevice() == ID_STATUS_LED)
		leds.setStatColor(pPacket->getData());
	else if(pPacket->getDevice() == ID_NINJA_EYES)
		leds.setEyesColor(pPacket->getData());
	else if(pPacket->getDevice() == ID_ONBOARD_RF)
	{
		m_Receiver.stop();
	
		CommonProtocolEncoder encoder(350);
		
		encoder.setCode(pPacket->getData());
		encoder.encode(&m_PacketTransmit);
		
		m_Transmitter.send(&m_PacketTransmit, 5);

		m_Receiver.start();
	}

	pPacket->setType(TYPE_ACK);
	pPacket->printToSerial();
}