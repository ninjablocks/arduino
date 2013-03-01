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

void NinjaLED::setStatColor(long nColor)
{
	m_nStatColor = nColor;

	analogWrite(RED_STAT_LED_PIN,   255 - (int((nColor & 0xff0000) >> 16)));
	analogWrite(GREEN_STAT_LED_PIN, 255 - (int((nColor & 0x00ff00) >> 8)));
	analogWrite(BLUE_STAT_LED_PIN,  255 - (int((nColor & 0x0000ff) >> 0)));
}

long NinjaLED::getStatColor()
{
	return m_nStatColor;
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
	int tempPORTB = PORTB;
	int tempPORTD = PORTD;

	digitalWrite(RED_STAT_LED_PIN, HIGH);
	digitalWrite(GREEN_STAT_LED_PIN, HIGH);
	digitalWrite(BLUE_STAT_LED_PIN, HIGH);

	delay(40);

	PORTB = tempPORTB;
	PORTD = tempPORTD;
}