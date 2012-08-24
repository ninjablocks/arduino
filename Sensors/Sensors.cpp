/*
	Sensors.cpp - Ninja Sensors library
	
	Developed by Ninja Blocks
	
	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

	Read README for version info.
	
*/

#include <Ninja.h>
#include <Arduino.h>
#include <Sensors.h>
#include <Wire.h>
#include <DHT22.h>
#include <NinjaObjects.h>

SENSORS Sensors;
DHT22 myDHT22 (14);
DHT22_ERROR_t errorCode;

extern NinjaObjects nOBJECTS;

SENSORS::SENSORS()
{
	_lastReadTime = millis()+60000;		// force TMP102 time checking to take 1st reading when started.
	_lastTemperature = 0;
}

int SENSORS::idTheType(int sensorValue, bool debug)
{
	if (debug){
		Serial.print("Sensor Value: ");
		Serial.println(sensorValue);
	}
	
	if (sensorValue > 178 && sensorValue < 198) { // Distance (ADC Value=188)
		return 10; 
	} else if (sensorValue > 250 && sensorValue < 270) { // PIR (ADC Value=260 )
			return 7;
	} else if (sensorValue > 327 && sensorValue < 347) { // 433Mhz Transmitter (ADC Value=337)
			return 1001;
	} else if (sensorValue > 365 && sensorValue < 385) { // Light (ADC Value=375)
			return 6;
	} else if (sensorValue > 415 && sensorValue < 435) { // 433Mhz Receiver (ADC Value=425)
			return 11;
	} else if (sensorValue > 511 && sensorValue < 531) { // Humidity & Temp (ADC Value=521)
			return 8;
	} else if (sensorValue > 559 && sensorValue < 579) { // Sound Sensor (ADC Value=569)
			return 12;
	} else if (sensorValue > 654 && sensorValue < 674) { // Relay Breakoutboard (ADC Value=664)
			return 1002;
	} else if (sensorValue > 782 && sensorValue < 802) { // Button (ADC value=792)
			return 5;
	} else if (sensorValue > 1010) { // Nothing
			return -1;
	} else {		// UNKNOWN sensor
			return 0;
	}
}

int SENSORS::getSensorValue(byte port, int deviceID)
{
	int aInPin=0;
	int dInPin=0;
	int sensorValue=0;
	static int tempValue;
	
	if(port == 1){
		aInPin = ADC_PIN_P1_1;
		dInPin = IO_PIN_P1_1;
	}
	else if (port == 2)
	{
		aInPin = ADC_PIN_P2_1;
		dInPin = IO_PIN_P2_1;
	}
	else if (port == 3)
	{
		aInPin = ADC_PIN_P3_1;
		dInPin = IO_PIN_P3_1;
	}
	else
	{
		//Serial.println("--> ERROR");
		//Serial.print("--> Attempting to assign port: ");
	}
	
	switch (deviceID)
	{
		case 0:
			// UNKNOWN Sensor, pass in the ADC Value
			pinMode(aInPin, INPUT);
			sensorValue = analogRead(aInPin);
			return sensorValue;
			
		case 5:
			// Push Button Sensor
			pinMode(dInPin, INPUT);
			sensorValue = digitalRead(dInPin);
			if (sensorValue==0)
				
#ifdef V11				
				nOBJECTS.blinkLED(BLUE_LED_PIN);
#endif

#ifdef V12				
				nOBJECTS.blinkLED(BLUE_STAT_LED_PIN);
#endif

			return sensorValue;
		
		case 6:
			// Light Sensor
			pinMode(aInPin, INPUT);
			sensorValue = analogRead(aInPin);
			return sensorValue;

		case 7:
			// PIR Sensor
			pinMode(dInPin, INPUT);
			sensorValue = digitalRead(dInPin);
			if (sensorValue>0)
				
#ifdef V11
				nOBJECTS.blinkLED(GREEN_LED_PIN);
#endif

#ifdef V12
				nOBJECTS.blinkLED(GREEN_STAT_LED_PIN);
#endif

			return sensorValue;

		case 8:
			//	DHT22 Humidity Sensor
			myDHT22.setPIN(dInPin);
			errorCode = myDHT22.readData();
			if ((errorCode==DHT_ERROR_NONE) || (errorCode==DHT_ERROR_TOOQUICK))
			{
				sensorValue = myDHT22.getHumidity() * 10 ;			// make a single decimal float to int
				tempValue = myDHT22.getTemperatureC() * 10;		// make a single decimal float to int
				return sensorValue;
			}
			else
				return 0;

		case 9:
			// DHT22 Temperature Sensor
			return tempValue;
		
		case 10:
			// Distance Sensor
			pinMode(aInPin, INPUT);
			sensorValue = analogRead(aInPin);
			return sensorValue;
		
		case 11:
			// 433Mhz Receiver - Not required to return anything
			// Special handling in doPort1()
			return -1;
			
		case 12:
			pinMode(aInPin, INPUT);
			sensorValue = analogRead(aInPin);
			if (sensorValue>0)

#ifdef V11
				nOBJECTS.blinkLED(RED_LED_PIN);
#endif

#ifdef V12
				nOBJECTS.blinkLED(RED_STAT_LED_PIN);
#endif

			return sensorValue;

		case 1002:
			// Relay Breakout
			pinMode(dInPin, OUTPUT);
			if (port == 1) 
				sensorValue = bitRead(PORTC, 0);
			else if (port == 2)
				sensorValue = bitRead(PORTC, 1);
			else if (port == 3)
				sensorValue = bitRead(PORTC, 2);
			
			return sensorValue;
						
		default:		// Invalid sensor ID
			return -1;
	}
}

#ifdef V11
float SENSORS::getBoardTemperature()
{
	unsigned long currentTime;
	
	currentTime = millis();
	// reading the TMP102 will heat up the device and cause a higher reading
	// slowing it down to 30sec per read.
	if (currentTime - _lastReadTime > 30000)		
	{
		_lastReadTime = currentTime;
		Wire.begin();
		Wire.requestFrom(TMP102_I2C_ADDRESS,2); 
		byte MSB = Wire.read();
		byte LSB = Wire.read();

		int TemperatureSum = ((MSB << 8) | LSB) >> 4; //it's a 12bit int, using two's compliment for negative
		float celsius = TemperatureSum*0.0625;
		celsius=celsius*10;
		int intTemp = (int)celsius;
		_lastTemperature = ((float)intTemp/10);
		return _lastTemperature;
	}
	else
		return _lastTemperature;
}
#endif
