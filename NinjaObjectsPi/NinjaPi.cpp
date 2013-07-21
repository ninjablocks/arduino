/*
	NinjaPi.cpp - Ninja Objects library
	
	Based on the aJson example from http://interactive-matter.eu/how-to/ajson-arduino-json-library/

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
#include <aJSON.h>
#include <MMA8453Q.h>
#include "NinjaPi.h"
#include <RCSwitch.h>

#include "NinjaDeviceIds.h"

// Variables need to be moved to private or public part of the object - TODO
//uint16_t biggest;
NinjaPi nOBJECTS;
char serInStr[recvLEN];  // array to hold the incoming serial string bytes

//extern char * const __brkval;
//extern struct __freelist *__flp;

char strGUID[GUID_LEN];
int intVID=0;
int intDID=0;
char strDATA[DATA_LEN];
int intDATA=0;
boolean IsDATAString=0;
RCSwitch mySwitch = RCSwitch();
boolean IgnoreHeartbeatDelay =false;
int _lastPort1ID=-1;
int _lastPort2ID=-1;
int _lastPort3ID=-1;

int iPort1Value=-1;
int iPort2Value=-1;
int iPort3Value=-1;


byte LightSensorCount=0;
int LightSensorTotalValue=0;

byte RED_LED_VALUE =0;
byte GREEN_LED_VALUE=0;
byte BLUE_LED_VALUE=0;

NinjaPi::NinjaPi()
{
	_lastHeartbeat = millis()+60000;
}

void NinjaPi::connectDevices(Device * customDevices[])
{
	this->customDevices = customDevices;
}

void NinjaPi::blinkLED(byte ledPin)
{
	int tempPORTB = PORTB;
	int tempPORTD = PORTD;
	
	digitalWrite(RED_STAT_LED_PIN, HIGH);
	digitalWrite(GREEN_STAT_LED_PIN, HIGH);
	digitalWrite(BLUE_STAT_LED_PIN, HIGH);
	digitalWrite(ledPin, LOW);

	delay(60);
	
	digitalWrite(RED_STAT_LED_PIN, HIGH);
	digitalWrite(GREEN_STAT_LED_PIN, HIGH);
	digitalWrite(BLUE_STAT_LED_PIN, HIGH);

	delay(20);

	PORTB = tempPORTB;
	PORTD = tempPORTD;
}

void dec2binWzerofill(char* bin, unsigned long long Dec, unsigned int bitLength){
//  static char bin[64]; 
	unsigned int i=0;

	while (Dec > 0) 
	{
		bin[32+i++] = ((Dec & 1) > 0) ? '1' : '0';
		Dec = Dec >> 1;
	}

	for (unsigned int j = 0; j< bitLength; j++) 
	{
		if (j >= bitLength - i) 
		{
			bin[j] = bin[ 31 + i - (j - (bitLength - i)) ];
		}else 
		{
			bin[j] = '0';
		}
	}
	bin[bitLength] = '\0';
	
	return;
}

//read a string from the serial and store it in an array
int readSerialString () 
{
	int i=0;
	if(!Serial.available()) 
		return -1;

	while (Serial.available()>0)
	{
		if( i < recvLEN) 				
		{
			char c = Serial.read();
			serInStr[i++] = c;
			delay(2);
		}
		else
			break;			
	}
	return i;
}

boolean NinjaPi::decodeJSON()
{
	boolean IsJSONValid=false;
	aJsonObject* rootObject = aJson.parse(serInStr);
	memset(serInStr,0,recvLEN);		// Clear serialBuffer

	if (rootObject != NULL)
	{
		if(aJson.getArraySize(rootObject)>0)
		{
			aJsonObject* devices = aJson.getObjectItem(rootObject,"DEVICE");
			if (devices != NULL)
			{
				
				aJsonObject* device = aJson.getArrayItem(devices,0);
				if (device !=NULL)
				{
					aJsonObject* jguid = aJson.getObjectItem(device,"G");
					aJsonObject* jvid = aJson.getObjectItem(device,"V");
					aJsonObject* jdid = aJson.getObjectItem(device,"D");
					aJsonObject* jdata = aJson.getObjectItem(device,"DA");
				
					if ((jguid != NULL) && (jguid->type == aJson_String)) 
					{
						strncpy(strGUID,jguid->valuestring,GUID_LEN);
								
						if ((jvid != NULL) && (jvid->type == aJson_Int))
						{
							intVID=jvid->valueint;
									
							if ((jdid != NULL) && (jdid->type == aJson_Int))
							{
								intDID=jdid->valueint;
								
								if (jdata != NULL)
								{
									if (jdata->type == aJson_String)
									{
										strncpy(strDATA, jdata->valuestring,DATA_LEN);
										IsJSONValid=true;
										IsDATAString=true;
									}
									else if (jdata->type == aJson_Int)
									{
										intDATA=jdata->valueint;
										IsJSONValid=true;
										IsDATAString=false;
									}
								}
							}
						}					
					}
				}
			}
		}

		aJson.deleteItem(rootObject);
	
		if(IsJSONValid)
			return true;
		else
			return false;  
	}
	else
		return false;
}

// Ninja reactor processing
void NinjaPi::doReactors()
{
	int spos = readSerialString();

	if (spos>0)
	{
		if(decodeJSON())
		{
			bool didUpdateDevice = checkDevicesForUpdate();
			if(!didUpdateDevice && (intVID==0))
			{
				switch (intDID)
				{
					case kNBDIDOnBoardStatusLED:
					{
						long colorVal = strtol(strDATA, NULL, 16);

						//RED_LED_VALUE = int((colorVal&0xff0000)>>16);
						analogWrite(RED_STAT_LED_PIN, 255-(int((colorVal&0xff0000)>>16)));

						//GREEN_LED_VALUE=int((colorVal&0x00ff00)>>8);
						analogWrite(GREEN_STAT_LED_PIN, 255-(int((colorVal&0x00ff00)>>8)));
				
						//BLUE_LED_VALUE = int((colorVal&0x0000ff)>>0);
						analogWrite(BLUE_STAT_LED_PIN, 255-(int((colorVal&0x0000ff)>>0)));	
				
						doJSONResponse();
						break;
					}
					
					case kNBDIDOnBoardRGBLED:
					{
						long colorVal = strtol(strDATA, NULL, 16);

						RED_LED_VALUE = int((colorVal&0xff0000)>>16);
						analogWrite(RED_LED_PIN, 255-RED_LED_VALUE );

						GREEN_LED_VALUE=int((colorVal&0x00ff00)>>8);
						analogWrite(GREEN_LED_PIN, 255-GREEN_LED_VALUE);
				
						BLUE_LED_VALUE = int((colorVal&0x0000ff)>>0);
						analogWrite(BLUE_LED_PIN, 255-BLUE_LED_VALUE);	
				
						doJSONResponse();
						break;
					}
					
					case 11:					// change this ID to 11 to accomodate all in one TX & RX board.
					{
						byte portPIN = atoi(strGUID);
						switch (portPIN)
						{
							case 1:
								mySwitch.enableTransmit(IO_PIN_P1_1);
								break;
							case 2:
								mySwitch.enableTransmit(IO_PIN_P2_1);
								break;
							case 3:
								mySwitch.enableTransmit(IO_PIN_P3_1);
								break;
							
							default:
								#if defined(V12) || defined(VRPI10)
								mySwitch.enableTransmit(TX433_PIN);
								#endif
								break;
						}
						mySwitch.setPulseLength(350);
						mySwitch.setRepeatTransmit(8);		// watch this, 3-8 repeats will be enough 
						mySwitch.send(strDATA);
						mySwitch.disableTransmit();
						doJSONResponse();
						break;
					}
					
					case 1003:			// Firmware version number
					{
						if (strcmp(strDATA,"VNO")==0)
						{
							strcpy(strDATA,"VRPi10_");		// need to find a way to generate new version number in sync with git tag - TODO
							strcat(strDATA,VERSION_NO);
							doJSONResponse();
						}
						else
							doJSONError(4);
						break;	
					}

					default:
						doJSONError(3);		// unknown device id
						break;		
				}
			}
			else
			// other vendors could add here
			// if (intVID==vendorID)
			// {
			// }				
				if (!didUpdateDevice) doJSONError(2);		// unknown vendor id
		}
		else
		{
			doJSONError(1);
		}
	}	
}

void NinjaPi::doJSONError(int errorCode)
{
	aJsonObject* root = aJson.createObject();
	if (root == NULL)
	{
		//Serial.println("error root"); 
		return;
	}

	aJsonObject* device = aJson.createArray();
	aJson.addItemToObject(root,"ERROR", device);
	
	aJsonObject* guid1 = aJson.createObject();
	aJson.addItemToArray(device, guid1);
	aJson.addNumberToObject(guid1, "CODE", errorCode);
	
	char* string = aJson.print(root);
	if (string != NULL) 
	{
		Serial.println(string);
	}

	aJson.deleteItem(root);
	free(string);
}

void NinjaPi::doJSONResponse()
{
	aJsonObject* root = aJson.createObject();
	if (root == NULL)
	{
		//Serial.println("error root"); 
		return;
	}

	aJsonObject* device = aJson.createArray();
	aJson.addItemToObject(root,"ACK", device);
	
	aJsonObject* guid1 = aJson.createObject();
	aJson.addItemToArray(device, guid1);
	aJson.addStringToObject(guid1, "G", strGUID);
	aJson.addNumberToObject(guid1, "V", intVID);
	aJson.addNumberToObject(guid1, "D", intDID);
	
	if(IsDATAString)
		aJson.addStringToObject(guid1, "DA", strDATA);
	else
		aJson.addNumberToObject(guid1, "DA", intDATA);

	char* string = aJson.print(root);
	if (string != NULL) 
	{
		Serial.println(string);
	}

	aJson.deleteItem(root);
	free(string);
}

void NinjaPi::doJSONData(const char * strGUID, int intVID, int intDID
	, const char * strDATA, double numDATA, bool IsString, byte dataTYPE)
{
	int tempDATA=0;
	
	if((dataTYPE<-1) || (dataTYPE>2))
		return;

	aJsonObject* root = aJson.createObject();
	if (root == NULL)
	{
		//Serial.println("error root"); 
		return;
	}

	aJsonObject* device = aJson.createArray();
	if (dataTYPE==0)
		aJson.addItemToObject(root,"DEVICE", device);
	
	if (dataTYPE==1)
		aJson.addItemToObject(root,"PLUGIN", device);
	
	if (dataTYPE==2)
		aJson.addItemToObject(root,"UNPLUG", device);
	
	aJsonObject* guid1 = aJson.createObject();
	aJson.addItemToArray(device, guid1);
	aJson.addStringToObject(guid1, "G", strGUID);
	aJson.addNumberToObject(guid1, "V", intVID);
	aJson.addNumberToObject(guid1, "D", intDID);
	
	if(IsString)
		aJson.addStringToObject(guid1, "DA", strDATA);
	else
	{
		tempDATA = (int)numDATA;
		if (numDATA!=tempDATA)
			aJson.addNumberToObject(guid1, "DA", numDATA);
		else
			aJson.addNumberToObject(guid1, "DA", tempDATA);
	}

	char* string = aJson.print(root);
	if (string != NULL) 
	{
		Serial.println(string);
	}

	aJson.deleteItem(root);
	free(string);
}

void NinjaPi::doOnBoardRGB() //including status LED
{
	char tempSTR[7];
	char tempColor[3];
	
	if (Serial.available()>0) doReactors();
	// Get RGB Value
	sprintf(tempColor,"%02X", RED_LED_VALUE);
	strcpy(tempSTR,tempColor);

	sprintf(tempColor,"%02X", GREEN_LED_VALUE);
	strcat(tempSTR,tempColor);
		
	sprintf(tempColor,"%02X", BLUE_LED_VALUE);
	strcat(tempSTR,tempColor);

	doJSONData("0", 0, kNBDIDOnBoardRGBLED, tempSTR, 0, true,0);
	
	// Get RGB STATUS LED Value
	int tempPORTB = PORTB & 0x11;		// Green & Red
	int tempPORTD = PORTD & 0x80;		// Blue
	
	if (tempPORTB & 0x10) 
		strcpy(tempSTR,"00");
	else
		strcpy(tempSTR,"FF");
		
	if(tempPORTB & 0x01)
		strcat(tempSTR,"00");
	else 
		strcat(tempSTR,"FF");
		
	if(tempPORTD)
		strcat(tempSTR,"00");
	else
		strcat(tempSTR,"FF");

	doJSONData("0", 0,kNBDIDOnBoardStatusLED, tempSTR,0,true,0);

	if (Serial.available()>0) doReactors();

}

void NinjaPi::doWT450(unsigned long long value)
{
	char strAddress[5];
	unsigned long data;
	int house=0;
	byte station=0;
	int humidity=0;
	double temperature=0;
	double tempdecimal=0;
	byte tempfraction=0;

	data=(unsigned long)value;

	house=(data>>28) & (0x0f);
	station=((data>>26) & (0x03))+1;
	humidity=(data>>16)&(0xff);
	temperature=((data>>8) & (0xff));
	temperature = temperature - 50;
	tempfraction=(data>>4) & (0x0f);

	tempdecimal=((tempfraction>>3 & 1) * 0.5) + ((tempfraction>>2 & 1) * 0.25) + ((tempfraction>>1 & 1) * 0.125) + ((tempfraction & 1) * 0.0625);
	temperature=temperature+tempdecimal;
	temperature=(int)(temperature*10);
	temperature=temperature/10;

	sprintf(strAddress,"%02d%02d", house, station);
	
	doJSONData(strAddress, 0, 30, NULL, humidity, false,0);
	doJSONData(strAddress, 0, 31, NULL, temperature, false,0);
	
}

void NinjaPi::doLacrosseTX3(unsigned long long tx3value)
{
	byte nibble[10];
	char strAddress[3];
	unsigned long long mask=0x0f;
	unsigned int checksum=0;

	for (int i=0;i<10;i++)
	{
		nibble[9-i]=0;
		nibble[9-i]=(byte)(tx3value & mask);
		if (i!=0) checksum=checksum+nibble[9-i];  // skip checksum byte
		tx3value=tx3value>>4;
	}
	
	checksum=checksum & 0xf;
	if ((nibble[0]==0xa) && (nibble[9]==checksum))
	{
		// good packet
		byte SensorAddress = (nibble[2]<<4) + ((nibble[3]&0xe)>>1);
		sprintf(strAddress,"%X", SensorAddress);
		unsigned int dataValue = ((int)(nibble[7]*100))+((int)(nibble[8]*10))+nibble[6];
		
		// temperature
		if (nibble[1]==0)
		{
			double realValue=dataValue;
			realValue=realValue/10;
			realValue-=50;
			doJSONData(strAddress, 0, 13, NULL, realValue, false,0);
		}
	}
}

void NinjaPi::doLacrosseWS2355(unsigned long long ws2344value)
{
	byte nibble[12];
	byte windir;
	unsigned long long mask=0x0f;
	unsigned int checksum=0;
	char strAddress[3];

	for (int i=0;i<12;i++)
	{
		nibble[11-i]=0;
		nibble[11-i]=(byte)(ws2344value & mask);
		if (i!=0) checksum=checksum+nibble[11-i];  // skip checksum byte
		ws2344value=ws2344value>>4;
	}

	checksum=checksum & 0xf;
	if ((nibble[0]==0x9) && (nibble[11]==checksum))
	{
		byte SensorAddress = (nibble[2]<<4) + nibble[3];
		sprintf(strAddress,"%X", SensorAddress);
		byte Type = nibble[1] & 0x3;

		switch(Type)
		{
			case 0:		// temperature in Celsius
				double temperature;
				temperature =nibble[6] *100;    
				temperature +=nibble[7] *10;
				temperature +=nibble[8];
				temperature =(temperature-300)/10;
				doJSONData(strAddress, 0, 20, NULL, temperature, false,0);
				break;

			case 1:		// humidity in %
				int humidity;
				humidity  = nibble[6]*10;
				humidity += nibble[7];
				doJSONData(strAddress, 0, 21, NULL, humidity, false,0);
				break;

			case 2:		// total rainfall in mm
				double rainfall;
				rainfall = nibble[6]<<8;
				rainfall += nibble[7]<<4;
				rainfall += nibble[8];
				rainfall = (rainfall*518)/1000;
				doJSONData(strAddress, 0, 22, NULL, rainfall, false,0);
				break;

			case 3:		// wind direction in NSEW and wind spped in KMH
				double windspeed;
				windir = (byte)nibble[8];
				windspeed = (nibble[5] & 0x1)<<8;
				windspeed += nibble[6]<<4;
				windspeed += nibble[7];
				windspeed =(windspeed/10)*3.6;
				//DEPRECATED: doJSONData(strAddress, 0, 23, (char *)strWindDirection[windir], 0, true,0);
				doJSONData(strAddress, 0, 23, NULL, windir, false, 0);
				doJSONData(strAddress, 0, 24, NULL, windspeed, false,0);
				break;

			default:
				break;
		}
	}
}

void NinjaPi::do433(void)
{
	int tempID;
	tempID=11;

	if (Serial.available()>0) doReactors();

	mySwitch.enableReceive(RX433_INT);

	if (mySwitch.available() && (mySwitch.getReceivedProtocol()>0 && mySwitch.getReceivedProtocol()<6))
	{
		unsigned long long value = mySwitch.getReceivedValue();
		if (value == 0) // unknown encoding
		{
			doJSONData("0", 0, tempID, "0", 0, true,0);
		} 
		else 
		{
			// Blink Green LED to show data valid
			blinkLED(GREEN_STAT_LED_PIN);

			if(mySwitch.getReceivedProtocol()==3) 
				doLacrosseTX3(value);
			else if (mySwitch.getReceivedProtocol()==4)
				doLacrosseWS2355(value);
			else if(mySwitch.getReceivedProtocol()==5)
				doWT450(value);
			else
			{		
				if (mySwitch.getReceivedBitlength()> (DATA_LEN/2))
					doJSONData("0", 0, tempID, "0", 0, true,0);
				else
				{
					dec2binWzerofill(strDATA, mySwitch.getReceivedValue(), mySwitch.getReceivedBitlength());
					doJSONData("0", 0, tempID, strDATA, 0, true,0);
				}
			}
		}
		mySwitch.resetAvailable();
	}
	//else
		//doJSONData("0", 0, tempID, "-1", 0, true,0);

	if (Serial.available()>0) doReactors();
}

//send custom device data to cloud
void NinjaPi::doCustomDevices() 
{
	Device ** iDevices = this->customDevices;
	while (*iDevices != (Device*)0) {
		this->doDevice(*iDevices);
		iDevices++;
	}
}

bool NinjaPi::checkDeviceForUpdate(Device *d) {
	bool didUpdateDevice = false;
	if((intVID == d->intVID) && (intDID == d->intDID)) { // same vendor and device id's
		size_t guidLen = strlen(strGUID);
		if ((guidLen == strlen(d->strGUID)) && (0 == strncmp(strGUID, d->strGUID, guidLen))) { // same guid
			if (d->IsString) {
				strncpy(d->strDATA, strDATA, d->strDATALenMax);
			} else {
				d->intDATA = intDATA;
			}
			d->didUpdate(d);
			didUpdateDevice = true;
		}
	}
	return didUpdateDevice;
}

bool NinjaPi::checkDevicesForUpdate()
{
	bool didUpdateDevice = false;
	Device ** iDevices = this->customDevices;
	while (*iDevices != (Device*)0) {
		didUpdateDevice |= this->checkDeviceForUpdate(*iDevices);
		iDevices++;
	}
	return didUpdateDevice;
}

void NinjaPi::sendObjects() 
{
	boolean IsDHT22=false;
	byte DHT22_PORT=0;

	do433();
	
	unsigned long currentHeartbeat = millis();
	// slow heart beat devices
	if((currentHeartbeat - _lastHeartbeat)>SLOW_DEVICE_HEARTBEAT)
	{
		doOnBoardRGB();
		doCustomDevices();
		_lastHeartbeat=currentHeartbeat;
	}
}
