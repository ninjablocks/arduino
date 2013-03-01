#include "Heartbeat.h"

Heartbeat heartbeat;

Heartbeat::Heartbeat()
{
	m_nTimeOut = millis() + 50000;
	m_nCurrent = 0;
}

void Heartbeat::check()
{
	m_nCurrent = millis();
}

void Heartbeat::resume()
{
	if(isExpired())
		m_nTimeOut = m_nCurrent + 30000;
}

bool Heartbeat::isExpired()
{
	return (m_nCurrent > m_nTimeOut);
}