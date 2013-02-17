#include <Arduino.h>
#include <inttypes.h>
#include <Ninja.h>
#include <DHT22.h>
//#include <I2C.h>
#include <aJSON.h>
#include <Sensors.h>
#include <NinjaObjects.h>
#include <RCSwitch.h>

void setup()
{
	Serial.begin(9600);
	Serial.println();
	pinMode(RED_LED_PIN, OUTPUT);
	pinMode(GREEN_LED_PIN, OUTPUT);
	pinMode(BLUE_LED_PIN, OUTPUT);
	digitalWrite(RED_LED_PIN, HIGH);            // set the RED LED  Off
	digitalWrite(GREEN_LED_PIN, HIGH);          // set the GREEN LED Off
	digitalWrite(BLUE_LED_PIN, HIGH);           // set the BLUE LED Off

#ifdef V11
        digitalWrite(BLUE_LED_PIN, LOW);           // Power on Status
#endif 
#ifdef V12
	pinMode(RED_STAT_LED_PIN, OUTPUT);
	pinMode(GREEN_STAT_LED_PIN, OUTPUT);
	pinMode(BLUE_STAT_LED_PIN, OUTPUT);
	digitalWrite(RED_STAT_LED_PIN, HIGH);		// set the RED STAT LED  Off
	digitalWrite(GREEN_STAT_LED_PIN, HIGH);	        // set the GREEN STAT LED Off
	digitalWrite(BLUE_STAT_LED_PIN, LOW);	        // Power on Status
#endif
}

void loop()
{
	nOBJECTS.sendObjects();				// Send Ninja Objects 
	nOBJECTS.doReactors();				// Receive Ninja Objects reactors
}