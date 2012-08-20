/*
	NinjaObjects.cpp - Ninja Objects library
	
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
#include <aJson.h>
#include <Sensors.h>
#include <MMA8453Q.h>
#include "NinjaObjects.h"
#include <RCSwitch.h>

const char strWindDirection[16][4] = 
{
	"N", "NNE", "NE", "ENE",
	"E", "ESE", "SE", "SSE",
	"S", "SSW", "SW", "WSW",
	"W", "WNW", "NW", "NNW"
};

struct __freelist
{
	size_t sz;
	struct __freelist *nx;
};


// Variables need to be moved to private or public part of the object - TODO
uint16_t biggest;
NinjaObjects nOBJECTS;
char serInStr[recvLEN];  // array to hold the incoming serial string bytes

extern char * const __brkval;
extern struct __freelist *__flp;

char strGUID[GUID_LEN];
int intVID=0;
int intDID=0;
char strDATA[DATA_LEN];
int intDATA=0;
boolean IsDATAString=0;
RCSwitch mySwitch = RCSwitch();
boolean IgnoreHeartbeatDelay =false;
int _lastPort1ID=0;
int _lastPort2ID=0;
int _lastPort3ID=0;

#ifdef V12
byte RED_LED_VALUE =0;
byte GREEN_LED_VALUE=0;
byte BLUE_LED_VALUE=0;
#endif

NinjaObjects::NinjaObjects()
{
	_lastHeartbeat = millis()+60000;
}

void NinjaObjects::blinkLED(byte ledPin)
{
	int tempPORTB = PORTB;
	int tempPORTD = PORTD;
	
	digitalWrite(RED_LED_PIN, HIGH);
	digitalWrite(GREEN_LED_PIN, HIGH);
	digitalWrite(BLUE_LED_PIN, HIGH);
	digitalWrite(ledPin, LOW);
	delay(60);
	
	digitalWrite(RED_LED_PIN, HIGH);
	digitalWrite(GREEN_LED_PIN, HIGH);
	digitalWrite(BLUE_LED_PIN, HIGH);
	delay(20);
	
	PORTB = tempPORTB;
	PORTD = tempPORTD;
}

int getIDPinReading(int pin)
{
	analogRead(pin);
	delay(10);
	int val = analogRead(pin);
	delay(20);
	return val;
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

// given a PROGMEM string, use Serial.print() to send it out
// this is needed to save precious memory
//thanks to todbot for this http://todbot.com/blog/category/programming/
void printProgStr(const prog_char* str) {
	char c;
	if (!str)
	{
		return;
	}
	while ((c = pgm_read_byte(str))) 
	{
		Serial.print(c, byte(0));
		str++;
	}
}

uint16_t freeMem(uint16_t *biggest)
{
	char *brkval;
	char *cp;
	unsigned freeSpace;
	struct __freelist *fp1, *fp2;

	brkval = __brkval;
	if (brkval == 0) 
	{
		brkval = __malloc_heap_start;
	}
	cp = __malloc_heap_end;
	if (cp == 0) 
	{
		cp = ((char *)AVR_STACK_POINTER_REG) - __malloc_margin;
	}
	if (cp <= brkval) return 0;

	freeSpace = cp - brkval;

	for (*biggest = 0, fp1 = __flp, fp2 = 0;fp1; fp2 = fp1, fp1 = fp1->nx) 
	{
		if (fp1->sz > *biggest) *biggest = fp1->sz;
		freeSpace += fp1->sz;
	}
	return freeSpace;
}

void freeMem(char* message) 
{
	Serial.print(message);
	Serial.print(":\t");
	Serial.println(freeMem(&biggest));
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

boolean NinjaObjects::decodeJSON()
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
void NinjaObjects::doReactors()
{
	int spos = readSerialString();

	if (spos>0)
	{
		if(decodeJSON())
		{
			if(intVID==0)  // VID 0 is reserved for Ninja Blocks
			{
				switch (intDID)
				{
					case 1000:		// On Board RGB Led
					{
						long colorVal = strtol(strDATA, NULL, 16);

#ifdef V11
						analogWrite(RED_LED_PIN, 255-int((colorVal&0xff0000)>>16) );
						analogWrite(GREEN_LED_PIN, 255-int((colorVal&0x00ff00)>>8) );
						analogWrite(BLUE_LED_PIN, 255-int((colorVal&0x0000ff)>>0) );			
#endif

#ifdef V12
						RED_LED_VALUE = int((colorVal&0xff0000)>>16);
						analogWrite(RED_LED_PIN, 255-RED_LED_VALUE );

						GREEN_LED_VALUE=int((colorVal&0x00ff00)>>8);
						analogWrite(GREEN_LED_PIN, 255-GREEN_LED_VALUE);
				
						BLUE_LED_VALUE = int((colorVal&0x0000ff)>>0);
						analogWrite(BLUE_LED_PIN, 255-BLUE_LED_VALUE);	
#endif
				
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
								#ifdef V12
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
					
					case 1002:
					{
						byte portPIN = atoi(strGUID);
						byte triggerPIN = -1;
						byte triggerDATA = -1;
						switch (portPIN)
						{
							case 1:
								triggerPIN = IO_PIN_P1_1;
								break;
							case 2:
								triggerPIN = IO_PIN_P2_1;
								break;
							case 3:
								triggerPIN = IO_PIN_P3_1;
								break;
							
							default:
								break;											
						}

						if (IsDATAString)
						{
							triggerDATA = atoi(strDATA);
						}
						else
						{
							triggerDATA = intDATA;
						}
						
						digitalWrite(triggerPIN, triggerDATA);
						doJSONResponse();	
						break;
					}
					
					case 1003:
					{
						if (strcmp(strDATA,"VNO")==0)
						{
				
#ifdef V11							
							strcpy(strDATA,"V11_");		// need to find a way to generate new version number in sync with git tag - TODO
#endif

#ifdef V12
							strcpy(strDATA,"V12_");		// need to find a way to generate new version number in sync with git tag - TODO
#endif

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
				doJSONError(2);		// unknown vendor id
		}
		else
		{
			doJSONError(1);
		}
	}	
}

void NinjaObjects::doJSONError(int errorCode)
{
	aJsonObject* root = aJson.createObject();
	if (root == NULL)
	{
		Serial.println("error root"); 
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

void NinjaObjects::doJSONResponse()
{
	aJsonObject* root = aJson.createObject();
	if (root == NULL)
	{
		Serial.println("error root"); 
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

void NinjaObjects::doJSONData(char * strGUID, int intVID, int intDID, char * strDATA, double numDATA, bool IsString)
{
	int tempDATA=0;
	aJsonObject* root = aJson.createObject();
	if (root == NULL)
	{
		Serial.println("error root"); 
		return;
	}

	aJsonObject* device = aJson.createArray();
	aJson.addItemToObject(root,"DEVICE", device);
	
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
		if (numDATA>tempDATA)
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

#ifdef V12
void NinjaObjects::doOnBoardRGB()
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

	doJSONData("0", 0, 1000, tempSTR, 0, true);
	if (Serial.available()>0) doReactors();

}
#endif

#ifdef V11
void NinjaObjects::doOnBoardRGB()
{
	char tempSTR[7];
	
	if (Serial.available()>0) doReactors();

	// Get RGB Value
	int tempPORTB = PORTB & 0x03;		// Green & Blue
	int tempPORTD = PORTD & 0x80;		// Red
	
	if (tempPORTD) 
		strcpy(tempSTR,"00");
	else
		strcpy(tempSTR,"FF");
		
	if(tempPORTB & 1)
		strcat(tempSTR,"00");
	else 
		strcat(tempSTR,"FF");
		
	if(tempPORTB & 2)
		strcat(tempSTR,"00");
	else
		strcat(tempSTR,"FF");

	doJSONData("0", 0,1000, tempSTR,0,true);

	if (Serial.available()>0) doReactors();
}

void NinjaObjects::doOnBoardTemp()
{
	float fTemperature=0;
	if (Serial.available()>0) doReactors();
	fTemperature =Sensors.getBoardTemperature();
	doJSONData("0", 0, 1, NULL, fTemperature, false);
	if (Serial.available()>0) doReactors();
}

void NinjaObjects::doOnBoardAccelerometer()
{
	char tempBuffer[15];
	int x = 0, y = 0, z = 0;
	
	if (Serial.available()>0) doReactors();

	MMA.getAccXYZ(&x, &y, &z, true); //get accelerometer readings in normal mode (hi res).
	sprintf(tempBuffer, "%d,%d,%d", x,y,z);

	doJSONData("0", 0, 2, tempBuffer, 0, true);
	if (Serial.available()>0) doReactors();
}
#endif

void NinjaObjects::doLacrosseTX3(unsigned long long tx3value)
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
			doJSONData(strAddress, 0, 13, NULL, realValue, false);
		}
	}
}

void NinjaObjects::doLacrosseWS2355(unsigned long long ws2344value)
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
				doJSONData(strAddress, 0, 20, NULL, temperature, false);
				break;

			case 1:		// humidity in %
				int humidity;
				humidity  = nibble[6]*10;
				humidity += nibble[7];
				doJSONData(strAddress, 0, 21, NULL, humidity, false);
				break;

			case 2:		// total rainfall in mm
				double rainfall;
				rainfall = nibble[6]<<8;
				rainfall += nibble[7]<<4;
				rainfall += nibble[8];
				rainfall = (rainfall*518)/1000;
				doJSONData(strAddress, 0, 22, NULL, rainfall, false);
				break;

			case 3:		// wind direction in NSEW and wind spped in KMH
				double windspeed;
				windir = (byte)nibble[8];
				windspeed = (nibble[5] & 0x1)<<8;
				windspeed += nibble[6]<<4;
				windspeed += nibble[7];
				windspeed =(windspeed/10)*3.6;
				doJSONData(strAddress, 0, 23, (char *)strWindDirection[windir], 0, true);
				doJSONData(strAddress, 0, 24, NULL, windspeed, false);
				break;

			default:
				break;
		}
	}
}

void NinjaObjects::do433(void)
{
	int tempID;
	tempID=11;

	if (Serial.available()>0) doReactors();

	mySwitch.enableReceive(RX433_INT);

	if (mySwitch.available()) 
	{
		unsigned long long value = mySwitch.getReceivedValue();
		if (value == 0) // unknown encoding
		{
#ifdef V11
			doJSONData("1", 0, tempID, "0", 0, true);
#endif

#ifdef V12			
			doJSONData("0", 0, tempID, "0", 0, true);
#endif			
		} 
		else 
		{
			// Blink Green LED to show data valid
			blinkLED(GREEN_LED_PIN);

			if(mySwitch.getReceivedProtocol()==3) 
				doLacrosseTX3(value);
			else if (mySwitch.getReceivedProtocol()==4)
				doLacrosseWS2355(value);
			else
			{		
				if (mySwitch.getReceivedBitlength()> (DATA_LEN/2))
#ifdef V11
					doJSONData("1", 0, tempID, "0", 0, true);
#endif

#ifdef V12					
					doJSONData("0", 0, tempID, "0", 0, true);
#endif
				else
				{
					dec2binWzerofill(strDATA, mySwitch.getReceivedValue(), mySwitch.getReceivedBitlength());
#ifdef V11
					doJSONData("1", 0, tempID, strDATA, 0, true);
#endif

#ifdef V12
					doJSONData("0", 0, tempID, strDATA, 0, true);
#endif
				}
			}
		}
		mySwitch.resetAvailable();
	}
	//else
		//doJSONData("0", 0, tempID, "-1", 0, true);

	if (Serial.available()>0) doReactors();
}

boolean NinjaObjects::doPort1(byte* DHT22_PORT)
{
	int tempID=0;
	
	boolean IsDHT22=false;
	*DHT22_PORT=0;
		
	if (Serial.available()>0) doReactors();
	
	// Checking Port 1
	tempID=Sensors.idTheType(getIDPinReading(ID_PIN_P1),false);

#ifdef V11
	if (tempID==11) 
		mySwitch.enableReceive(RX433_INT);
	else
		mySwitch.disableReceive();
#endif
		
	if (tempID==8)
	{
		IsDHT22=true; 
		*DHT22_PORT=1;
		if (_lastPort1ID != tempID)
			IgnoreHeartbeatDelay=true;
	}
	else
	{
		if (tempID>-1)
		{
			if (tempID==0)
				doJSONData("1", 0, tempID, NULL, getIDPinReading(ID_PIN_P1), false);
#ifdef V11
			else if(tempID==11)	// 433Mhz Receiver
			{
				if(_lastPort1ID != tempID)	
					doJSONData("1",0,tempID, "-1",0,true);
				do433();
			}
#endif
		else				
			doJSONData("1", 0, tempID, NULL, Sensors.getSensorValue(1, tempID), false);
		}
	}
	_lastPort1ID=tempID;
	if (Serial.available()>0) doReactors();
	return IsDHT22;
}

boolean NinjaObjects::doPort2(byte* DHT22_PORT)
{
	int tempID=0;

	boolean IsDHT22=false;
	*DHT22_PORT=0;
			
	if (Serial.available()>0) doReactors();
	// Checking Port 2
	tempID=Sensors.idTheType(getIDPinReading(ID_PIN_P2),false);
	if (tempID==8) 
	{
		IsDHT22=true; 
		*DHT22_PORT=2;
		if (_lastPort2ID != tempID)
			IgnoreHeartbeatDelay=true;
	}
	else
	{
		if ((tempID>-1) && (tempID !=11))
		{
			if (tempID==0)
				doJSONData("2", 0, tempID, NULL, getIDPinReading(ID_PIN_P2),false);
			else
			{
				doJSONData("2", 0, tempID, NULL, Sensors.getSensorValue(2, tempID),false);
			}
		}
	}
	if (Serial.available()>0) doReactors();
	_lastPort2ID=tempID;
	return IsDHT22;
}

boolean NinjaObjects::doPort3(byte* DHT22_PORT)
{
	int tempID=0;
	boolean IsDHT22=false;
	*DHT22_PORT=0;	
			
	if (Serial.available()>0) doReactors();			
	// Checking Port 3
	tempID=Sensors.idTheType(getIDPinReading(ID_PIN_P3),false);
	if (tempID==8) 
	{
		IsDHT22=true; 
		*DHT22_PORT=3;
		if (_lastPort3ID != tempID)
				IgnoreHeartbeatDelay=true;
	}
	else
	{
		if ((tempID>-1) && (tempID !=11))
		{
			if (tempID==0)
				doJSONData("3", 0, tempID, NULL, getIDPinReading(ID_PIN_P3), false);
			else
			{
				doJSONData("3", 0, tempID, NULL, Sensors.getSensorValue(3, tempID), false);
			}
		}
	}
	_lastPort3ID=tempID;
	if (Serial.available()>0) doReactors();
	return IsDHT22;
}

void NinjaObjects::doDHT22(byte port)
{
	int intTemperature;
	int intHumidity;
	
	char tempBuffer[15];
	if (Serial.available()>0) doReactors();
	sprintf(tempBuffer, "%d", port);
	
	intHumidity =Sensors.getSensorValue(port, 8);
	intTemperature = Sensors.getSensorValue(port, 9);
	
	if ((intHumidity>0) && (intHumidity<2000))
	{
		doJSONData(tempBuffer, 0, 8, NULL, (float)intHumidity/10, false);
		IgnoreHeartbeatDelay=false;		// Successfully sent, now obey heartbeat

		if ((intTemperature>0) && (intTemperature<2000))
		{
			doJSONData(tempBuffer, 0,9,NULL,(float)intTemperature/10, false);
		}
	}
	
	if (Serial.available()>0) doReactors();
}

void NinjaObjects::sendObjects() 
{
	boolean IsDHT22=false;
	byte DHT22_PORT=0;

#ifdef V11
	doOnBoardAccelerometer();
#endif

#ifdef V12
	do433();
#endif

	IsDHT22=doPort1(&DHT22_PORT);

	unsigned long currentHeartbeat = millis();

	if (IsDHT22) 
		if((IgnoreHeartbeatDelay) || ((currentHeartbeat - _lastHeartbeat)>SLOW_DEVICE_HEARTBEAT)) doDHT22(DHT22_PORT);

	IsDHT22=doPort2(&DHT22_PORT);
	
	if (IsDHT22) 
		if((IgnoreHeartbeatDelay) || ((currentHeartbeat - _lastHeartbeat)>SLOW_DEVICE_HEARTBEAT)) doDHT22(DHT22_PORT);
	
	IsDHT22=doPort3(&DHT22_PORT);

	if (IsDHT22) 
		if((IgnoreHeartbeatDelay) || ((currentHeartbeat - _lastHeartbeat)>SLOW_DEVICE_HEARTBEAT)) doDHT22(DHT22_PORT);

// slow heart beat devices
	if((currentHeartbeat - _lastHeartbeat)>SLOW_DEVICE_HEARTBEAT)
	{
		doOnBoardRGB();
	
	#ifdef V11
		doOnBoardTemp();
	#endif

	_lastHeartbeat=currentHeartbeat;
	}
}
