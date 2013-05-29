#include "NinjaLED.h"

#define RED_LED_PIN			10
#define GREEN_LED_PIN		11
#define BLUE_LED_PIN		9

#define RED_STAT_LED_PIN 	12
#define GREEN_STAT_LED_PIN 	8
#define BLUE_STAT_LED_PIN 	7

NinjaLED::NinjaLED()
{
}

void NinjaLED::setup()
{
	pinMode(RED_LED_PIN, OUTPUT);
	pinMode(GREEN_LED_PIN, OUTPUT);
	pinMode(BLUE_LED_PIN, OUTPUT);
	
	digitalWrite(RED_LED_PIN, HIGH);
	digitalWrite(GREEN_LED_PIN, HIGH);
	digitalWrite(BLUE_LED_PIN, HIGH);

	pinMode(RED_STAT_LED_PIN, OUTPUT);
	pinMode(GREEN_STAT_LED_PIN, OUTPUT);
	pinMode(BLUE_STAT_LED_PIN, OUTPUT);
	
	digitalWrite(RED_STAT_LED_PIN, HIGH);
	digitalWrite(GREEN_STAT_LED_PIN, HIGH);
	digitalWrite(BLUE_STAT_LED_PIN, LOW);

	m_nEyesColor = 0;
	m_nStatColor = 0x0000FF;
}

void NinjaLED::timerSetup()
{
  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;

  int timer1_counter = 63536;   // preload timer 65536-16MHz/8/1kHz
  
  TCNT1 = timer1_counter;   // preload timer
  //TCCR1B |= (1 << CS11);    // 8 prescaler 
  TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
  interrupts();             // enable all interrupts
  blinkyEnabled = 0;			//initially no blinking
}

void NinjaLED::setEyesColor(long nColor)
{
	m_nEyesColor = nColor;

	analogWrite(RED_LED_PIN,   255 - (int((nColor & 0xff0000) >> 16)));
	analogWrite(GREEN_LED_PIN, 255 - (int((nColor & 0x00ff00) >> 8)));
	analogWrite(BLUE_LED_PIN,  255 - (int((nColor & 0x0000ff) >> 0)));
}

long NinjaLED::getEyesColor()
{
	return m_nEyesColor;
}

void NinjaLED::setStatColor(unsigned long long nColor)
{

	analogWrite(RED_STAT_LED_PIN,   255 - (int((nColor & 0xff0000) >> 16)));
	analogWrite(GREEN_STAT_LED_PIN, 255 - (int((nColor & 0x00ff00) >> 8)));
	analogWrite(BLUE_STAT_LED_PIN,  255 - (int((nColor & 0x0000ff) >> 0)));
	
	m_nStatColor = (unsigned long)((nColor<<40)>>40);
	unsigned int tempPeriod = (unsigned int)((nColor<<24)>>48);
	byte tempDutyCycle = (byte)((nColor<<16)>>56);
	
	if(tempPeriod != 0)
	{
		if(tempDutyCycle == 0)
			disableBlinky();
		else
		{
			if (!blinkyEnabled) enableBlinky();
			m_nPeriod = tempPeriod;
			double result = (tempDutyCycle*(double)tempPeriod)/256;
			m_nDutyCycle = (unsigned int)result;
		}
	
	}
}

long NinjaLED::getStatColor()
{
	unsigned long long tempColor;
	if(blinkyEnabled && m_nStatColor == 0) tempColor = m_nTempStatColor;
	else tempColor = m_nStatColor;
	
	return tempColor;
}

void NinjaLED::blinkEyes()
{
	int tempPORTB = PORTB;
	int tempPORTD = PORTD;

	digitalWrite(RED_LED_PIN, HIGH);
	digitalWrite(GREEN_LED_PIN, HIGH);
	digitalWrite(BLUE_LED_PIN, HIGH);

	delay(40);

	PORTB = tempPORTB;
	PORTD = tempPORTD;
}

void NinjaLED::blinkStat()
{
	disableBlinky();
	int tempPORTB = PORTB;
	int tempPORTD = PORTD;

	digitalWrite(RED_STAT_LED_PIN, HIGH);
	digitalWrite(GREEN_STAT_LED_PIN, HIGH);
	digitalWrite(BLUE_STAT_LED_PIN, HIGH);

	delay(40);

	PORTB = tempPORTB;
	PORTD = tempPORTD;
	enableBlinky();
}

void NinjaLED::statToggle()
{
	if (m_nStatColor > 0) 	//LED On
	{
		m_nTempStatColor = m_nStatColor;
		setStatColor(0x000000);
	}
	else
		setStatColor(m_nTempStatColor);
}

void NinjaLED::statOn()
{
	if (m_nStatColor > 0) 	//LED On
	{
		m_nTempStatColor = m_nStatColor;
	}
	else
		setStatColor(m_nTempStatColor);
}

void NinjaLED::statOff()
{
		m_nTempStatColor = m_nStatColor;
		setStatColor(0x000000);
}


void NinjaLED::enableBlinky()
{
	  blinkyEnabled = 1;
	  TCCR1B |= (1 << CS11);    // 8 prescaler
	  
}

void NinjaLED::disableBlinky()
{
	  blinkyEnabled = 0;
	  TCCR1B &= ~(1 << CS11);    // 0 prescaler - Disables timer
	  
}

/*void NinjaLED::enableBlinky(int period, int dutyCycle)
{
	m_nPeriod = period;
	m_nDutyCycle = dutyCycle;
	TCCR1B |= (1 << CS11);    // 8 prescaler this starts the timer
}*/