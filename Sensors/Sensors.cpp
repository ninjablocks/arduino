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


	Version 0.2: 5th May 2012 by JP Liew 
		- moved to Sensors library
		
	Version 0.1: by Marcus Schappi www.ninjablocks.com

*/

#include <Ninja.h>
#include <Arduino.h>
#include <Sensors.h>
#include <Wire.h>
#include <DHT22.h>

SENSORS Sensors;
DHT22 myDHT22 (14);
DHT22_ERROR_t errorCode;

SENSORS::SENSORS()
{

}

int SENSORS::idTheType(int sensorValue, bool debug)
{
	if (debug){
		Serial.print("Sensor Value: ");
		Serial.println(sensorValue);
	}
	
	if (sensorValue > 178 && sensorValue < 198) { // Distance (ADC Value=188)
		return 10; 
	} else if (sensorValue > 511 && sensorValue < 531) { // Humidity & Temp (ADC Value=521)
			return 8;
	} else if (sensorValue > 250 && sensorValue < 270) { // PIR (ADC Value=260 )
			return 7;
	} else if (sensorValue > 365 && sensorValue < 385) { // Light (ADC Value=375)
			return 6;
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
	int aInPin;
	int dInPin;
	int sensorValue;
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
		case 5:
			// Push Button Sensor
			sensorValue = digitalRead(dInPin) * 1023;
			return sensorValue;
		
		case 6:
			// Light Sensor
			sensorValue = analogRead(aInPin);
			return sensorValue;

		case 7:
			// PIR Sensor
			sensorValue = digitalRead(dInPin) * 1023;
			return sensorValue;

		case 8:
			//	DHT22 Humidity Sensor
			myDHT22.setPIN(dInPin);
			errorCode = myDHT22.readData();
			if ((errorCode==DHT_ERROR_NONE) || (errorCode==DHT_ERROR_TOOQUICK))
			{
				sensorValue = myDHT22.getHumidity() * 10;			// make a single decimal float to int
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
			sensorValue = analogRead(aInPin);
			return sensorValue;
			
		default:		// Invalid sensor ID
			return -1;
	}
		
/*	
	if(type == "LIGHT")
	{
		sensorValue = analogRead(aInPin);
		return sensorValue;
	}
	else if(type == "DISTANCE")
	{
		sensorValue = analogRead(aInPin);
		return sensorValue;
	}
	else if(type == "HUMIDITY")
	{
		myDHT22.setPIN(dInPin);
		errorCode = myDHT22.readData();
		if ((errorCode==DHT_ERROR_NONE) || (errorCode==DHT_ERROR_TOOQUICK))
			return myDHT22.getHumidity();
		else
			return 0;
	}
	else if(type == "BUTTON")
	{
		//pinMode(dInPin, INPUT);
		sensorValue = digitalRead(dInPin) * 1023;
		return sensorValue;
	}
	else if(type == "PIR")
	{
		sensorValue = digitalRead(dInPin) * 1023;
		return sensorValue;
	}
	else
	{
		return 0;
	}
	*/
	
}

float SENSORS::getBoardTemperature()
{
  Wire.begin();
  Wire.requestFrom(TMP102_I2C_ADDRESS,2); 
  byte MSB = Wire.read();
  byte LSB = Wire.read();

  int TemperatureSum = ((MSB << 8) | LSB) >> 4; //it's a 12bit int, using two's compliment for negative
  float celsius = TemperatureSum*0.0625;

  return celsius;
}

