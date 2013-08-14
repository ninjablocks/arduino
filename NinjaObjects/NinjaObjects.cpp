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

#include "NinjaDeviceIds.h"

//DEPRECATED
// const char strWindDirection[16][4] = 
// {
// 	"N", "NNE", "NE", "ENE",
// 	"E", "ESE", "SE", "SSE",
// 	"S", "SSW", "SW", "WSW",
// 	"W", "WNW", "NW", "NNW"
//};

/*
struct __freelist
{
	size_t sz;
	struct __freelist *nx;
};
*/

// Variables need to be moved to private or public part of the object - TODO
//uint16_t biggest;
NinjaObjects nOBJECTS;
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

#ifdef TESTER
boolean Enable433Receive =false;
#endif

#if defined(V12) || defined(VRPI10)
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
	
#ifdef V11
	digitalWrite(RED_LED_PIN, HIGH);
	digitalWrite(GREEN_LED_PIN, HIGH);
	digitalWrite(BLUE_LED_PIN, HIGH);
	digitalWrite(ledPin, LOW);
#endif

#if defined(V12) || defined(VRPI10)
	digitalWrite(RED_STAT_LED_PIN, HIGH);
	digitalWrite(GREEN_STAT_LED_PIN, HIGH);
	digitalWrite(BLUE_STAT_LED_PIN, HIGH);
	digitalWrite(ledPin, LOW);
#endif

	delay(60);
	
#ifdef V11
	digitalWrite(RED_LED_PIN, HIGH);
	digitalWrite(GREEN_LED_PIN, HIGH);
	digitalWrite(BLUE_LED_PIN, HIGH);
#endif
	
#if defined(V12) || defined(VRPI10)
	digitalWrite(RED_STAT_LED_PIN, HIGH);
	digitalWrite(GREEN_STAT_LED_PIN, HIGH);
	digitalWrite(BLUE_STAT_LED_PIN, HIGH);
#endif

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


/*
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
*/

/*
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
*/


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
					case kNBDIDOnBoardStatusLED:
					{
						long colorVal = strtol(strDATA, NULL, 16);

#ifdef V11
						analogWrite(RED_LED_PIN, 255-int((colorVal&0xff0000)>>16) );
						analogWrite(GREEN_LED_PIN, 255-int((colorVal&0x00ff00)>>8) );
						analogWrite(BLUE_LED_PIN, 255-int((colorVal&0x0000ff)>>0) );			
#endif

#if defined(V12) || defined(VRPI10)
						//RED_LED_VALUE = int((colorVal&0xff0000)>>16);
						analogWrite(RED_STAT_LED_PIN, 255-(int((colorVal&0xff0000)>>16)));

						//GREEN_LED_VALUE=int((colorVal&0x00ff00)>>8);
						analogWrite(GREEN_STAT_LED_PIN, 255-(int((colorVal&0x00ff00)>>8)));
				
						//BLUE_LED_VALUE = int((colorVal&0x0000ff)>>0);
						analogWrite(BLUE_STAT_LED_PIN, 255-(int((colorVal&0x0000ff)>>0)));	
#endif
				
						doJSONResponse();
						break;
					}
					
					case kNBDIDOnBoardRGBLED:
					{
						long colorVal = strtol(strDATA, NULL, 16);

#ifdef V11
						analogWrite(RED_LED_PIN, 255-int((colorVal&0xff0000)>>16) );
						analogWrite(GREEN_LED_PIN, 255-int((colorVal&0x00ff00)>>8) );
						analogWrite(BLUE_LED_PIN, 255-int((colorVal&0x0000ff)>>0) );			
#endif

#if defined(V12) || defined(VRPI10)
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
					
					case 1002:				// Relay
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
					
					case 1003:			// Firmware version number
					{
						if (strcmp(strDATA,"VNO")==0)
						{
				
#ifdef V11							
							strcpy(strDATA,"V11_");		// need to find a way to generate new version number in sync with git tag - TODO
#endif

#ifdef V12
							strcpy(strDATA,"V12_");		// need to find a way to generate new version number in sync with git tag - TODO
#endif

#ifdef VRPI10
							strcpy(strDATA,"VRPi10_");		// need to find a way to generate new version number in sync with git tag - TODO
#endif
							strcat(strDATA,VERSION_NO);
							doJSONResponse();
						}
						else
							doJSONError(4);
						break;	
					}

#ifdef TESTER
					case 8888:			// Tester Read IO command
						if (intVID==0)
						{
							byte portPIN = atoi(strGUID);
							byte portDATA =0;
							int analogDATA=0;
							switch (portPIN)
							{
								case 1:
									pinMode(IO_PIN_P1_1, INPUT);
									pinMode(IO_PIN_P1_2, INPUT);
									pinMode(IO_PIN_P1_3, INPUT);
									delay(10);
									portDATA = digitalRead(IO_PIN_P1_1)<<2;
									portDATA |= digitalRead(IO_PIN_P1_2)<<1;
									portDATA |= digitalRead(IO_PIN_P1_3);
									intDATA=portDATA;
									doJSONResponse();
									break; 
							
								case 2:
									pinMode(IO_PIN_P2_1, INPUT);
									pinMode(IO_PIN_P2_2, INPUT);
									pinMode(IO_PIN_P2_3, INPUT);
									delay(10);
									portDATA = digitalRead(IO_PIN_P2_1)<<2;
									analogDATA = analogRead(IO_PIN_P2_2);
									if (analogDATA>1010)
										portDATA |= 1 <<1;
									portDATA |= digitalRead(IO_PIN_P2_3);
									intDATA=portDATA;
									doJSONResponse();
									break; 
									
								case 3:
									pinMode(IO_PIN_P3_1, INPUT);
									pinMode(IO_PIN_P3_2, INPUT);
									pinMode(IO_PIN_P3_3, INPUT);
									delay(10);
									portDATA = digitalRead(IO_PIN_P3_1)<<2;
									analogDATA = analogRead(IO_PIN_P3_2);
									if (analogDATA>1010)
										portDATA |= 1 <<1;
									portDATA |= digitalRead(IO_PIN_P3_3);
									intDATA=portDATA;
									doJSONResponse();
									break; 
								
								default:
									doJSONError(3);
							}
						}
						else
							doJSONError(3);
						break;
					
					case 8889:
						if (intVID==0)
						{
							byte portPIN = atoi(strGUID);
							byte portDATA =0;
							switch (portPIN)
							{
								case 1:
									pinMode(IO_PIN_P1_1, OUTPUT);
									pinMode(IO_PIN_P1_2, OUTPUT);
									pinMode(IO_PIN_P1_3, OUTPUT);
									delay(10);
									digitalWrite(IO_PIN_P1_1,(intDATA &  4)>>2);
									digitalWrite(IO_PIN_P1_2,(intDATA &  2)>>1);
									digitalWrite(IO_PIN_P1_3,(intDATA &  1));
									doJSONResponse();
									break;
								
								default:
									doJSONError(3);
							}
						}
						else
							doJSONError(3);
						break;
						
					case 8890:
						if (intVID==0)
						{
							byte portDATA =0;
							
							if (IsDATAString)
							{
								portDATA = atoi(strDATA);
							}
							else
							{
								portDATA = intDATA;
							}
							
							if (portDATA==0)
							{
								Enable433Receive=false;
								mySwitch.disableReceive();
							}
							
							if (portDATA==1)
							{
								Enable433Receive=true;
							}
								
							
							doJSONResponse();
						}
						else
							doJSONError(3);
						break;
						
#endif
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

void NinjaObjects::doJSONResponse()
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

void NinjaObjects::doJSONData(char * strGUID, int intVID, int intDID, char * strDATA, double numDATA, bool IsString, byte dataTYPE)
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

#if defined(V12) || defined(VRPI10)
void NinjaObjects::doOnBoardRGB() //including status LED
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

	doJSONData("0", 0,kNBDIDOnBoardRGBLED, tempSTR,0,true,0);

	if (Serial.available()>0) doReactors();
}

void NinjaObjects::doOnBoardTemp()
{
	float fTemperature=0;
	if (Serial.available()>0) doReactors();
	fTemperature =Sensors.getBoardTemperature();
	doJSONData("0", 0, 1, NULL, fTemperature, false,0);
	if (Serial.available()>0) doReactors();
}

void NinjaObjects::doOnBoardAccelerometer()
{
	char tempBuffer[15];
	int x = 0, y = 0, z = 0;
	
	if (Serial.available()>0) doReactors();

	MMA.getAccXYZ(&x, &y, &z, true); //get accelerometer readings in normal mode (hi res).
	sprintf(tempBuffer, "%d,%d,%d", x,y,z);

	doJSONData("0", 0, 2, tempBuffer, 0, true,0);
	if (Serial.available()>0) doReactors();
}
#endif

void NinjaObjects::doWT450(unsigned long long value)
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
			doJSONData(strAddress, 0, 13, NULL, realValue, false,0);
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

unsigned long long previousValue = 0;
unsigned long previousTime = 0;
//WattsClever socket's remote binary codes pulsed out every ~300ms. temp/humidty sensor ~500ms
#define RF_DEBOUNCE_MILLIS 700
//WattsClever socket's remote binary codes vary by the 4th bit, ie 8.  Near identical temp/humidity values vary by around 3-5
#define RF_VALUE_TOLERANCE 7
void NinjaObjects::do433(void)
{
	int tempID;
	tempID=11;

	if (Serial.available()>0) doReactors();

	mySwitch.enableReceive(RX433_INT);

	if (mySwitch.available() && (mySwitch.getReceivedProtocol()>0 && mySwitch.getReceivedProtocol()<6))
	{
		unsigned long long value = mySwitch.getReceivedValue();
		unsigned long long diff = value - previousValue;
		if (value < previousValue) { diff = previousValue - value; }
		unsigned long timeReceived = millis();
		if (value == 0) // unknown encoding
		{
#ifdef V11
			doJSONData("1", 0, tempID, "0", 0, true,0);
#endif

#if defined(V12) || defined(VRPI10)
			doJSONData("0", 0, tempID, "0", 0, true,0);
#endif			
		} 
		else if ((timeReceived > (previousTime + RF_DEBOUNCE_MILLIS)) // time is after debounce period from previous reading
			 || (diff > RF_VALUE_TOLERANCE) // or it is a different value
			 )
		{
			previousValue = value;
			previousTime = timeReceived;

			// Blink Green LED to show data valid
#ifdef V11
			blinkLED(GREEN_LED_PIN);
#endif

#if defined(V12) || defined(VRPI10)
			blinkLED(GREEN_STAT_LED_PIN);
#endif

			if(mySwitch.getReceivedProtocol()==3) 
				doLacrosseTX3(value);
			else if (mySwitch.getReceivedProtocol()==4)
				doLacrosseWS2355(value);
			else if(mySwitch.getReceivedProtocol()==5)
				doWT450(value);
			else
			{		
				if (mySwitch.getReceivedBitlength()> (DATA_LEN/2))
#ifdef V11
					doJSONData("1", 0, tempID, "0", 0, true,0);
#endif

#if defined(V12) || defined(VRPI10)
					doJSONData("0", 0, tempID, "0", 0, true,0);
#endif
				else
				{
					dec2binWzerofill(strDATA, mySwitch.getReceivedValue(), mySwitch.getReceivedBitlength());
#ifdef V11
					doJSONData("1", 0, tempID, strDATA, 0, true,0);
#endif

#if defined(V12) || defined(VRPI10)
					doJSONData("0", 0, tempID, strDATA, 0, true,0);
#endif
				}
			}
		}
		mySwitch.resetAvailable();
	}
	//else
		//doJSONData("0", 0, tempID, "-1", 0, true,0);

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

	if ((_lastPort1ID == -1) && (tempID>-1))
	{
		iPort1Value=-1;
		doJSONData("1", 0, tempID, NULL, 0, false, 1);
		if (tempID==6)
		{
			LightSensorCount=0;
			LightSensorTotalValue=0;		
		}
	}
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
				doJSONData("1", 0, tempID, NULL, getIDPinReading(ID_PIN_P1), false,0);
#ifdef V11
			else if(tempID==11)	// 433Mhz Receiver
			{
				if(_lastPort1ID != tempID)	
					doJSONData("1",0,tempID, "-1",0,true,0);
				do433();
			}
#endif
			else
			{
				int tempPortValue = Sensors.getSensorValue(1, tempID);
				
				if (tempID==6)
				{
					LightSensorTotalValue+=tempPortValue;
					LightSensorCount++;
					if (LightSensorCount>9)
					{
						double tempLightValue =  (LightSensorTotalValue/10)*0.0977;
						tempLightValue = (int)(tempLightValue*10);
						tempLightValue = tempLightValue/10;
						doJSONData("1", 0, tempID, NULL, tempLightValue, false,0);
						LightSensorCount=0;
						LightSensorTotalValue=0;
					}
				} else if((tempID==5) || (tempID==7) || (tempID==1002))
				{
					if (iPort1Value != tempPortValue)
						doJSONData("1", 0, tempID, NULL, tempPortValue, false,0);
				}
				else 
					doJSONData("1", 0, tempID, NULL, tempPortValue, false,0);
						
				iPort1Value=tempPortValue;
			}
		}
		else if(_lastPort1ID>-1)
			doJSONData("1", 0, _lastPort1ID, NULL, 0, false, 2);
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

	if ((_lastPort2ID == -1) && (tempID>-1))
	{
		iPort2Value=-1;
		doJSONData("2", 0, tempID, NULL, 0, false, 1);
		if (tempID==6)
		{
			LightSensorCount=0;
			LightSensorTotalValue=0;		
		}
	}
	
	if (tempID==8) 
	{
		IsDHT22=true; 
		*DHT22_PORT=2;
		if (_lastPort2ID != tempID)
			IgnoreHeartbeatDelay=true;
	}
	else
	{
		if (tempID !=11)
		{
			if (tempID>-1)
			{
				if (tempID==0)
					doJSONData("2", 0, tempID, NULL, getIDPinReading(ID_PIN_P2),false,0);
				else
				{
					int tempPortValue = Sensors.getSensorValue(2, tempID);
				
					if (tempID==6)
					{
						LightSensorTotalValue+=tempPortValue;
						LightSensorCount++;
						if (LightSensorCount>9)
						{
							double tempLightValue =  (LightSensorTotalValue/10)*0.0977;
							tempLightValue = (int)(tempLightValue*10);
							tempLightValue = tempLightValue/10;
							doJSONData("2", 0, tempID, NULL, tempLightValue, false,0);
							LightSensorCount=0;
							LightSensorTotalValue=0;
						}
					} else if((tempID==5) || (tempID==7) || (tempID==1002))
					{
						if (iPort2Value != tempPortValue)
							doJSONData("2", 0, tempID, NULL, tempPortValue, false,0);
					}
					else 
						doJSONData("2", 0, tempID, NULL, tempPortValue, false,0);
						
					iPort2Value=tempPortValue;
				}
			}
			else if(_lastPort2ID>-1)
				doJSONData("2", 0, _lastPort2ID, NULL, 0, false, 2);
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

	if ((_lastPort3ID == -1) && (tempID>-1))
	{
		iPort3Value=-1;
		doJSONData("3", 0, tempID, NULL, 0, false, 1);
		if (tempID==6)
		{
			LightSensorCount=0;
			LightSensorTotalValue=0;		
		}
	}

	if (tempID==8) 
	{
		IsDHT22=true; 
		*DHT22_PORT=3;
		if (_lastPort3ID != tempID)
				IgnoreHeartbeatDelay=true;
	}
	else
	{
		if(tempID !=11)
		{
			if (tempID>-1)
			{
				if (tempID==0)
					doJSONData("3", 0, tempID, NULL, getIDPinReading(ID_PIN_P3), false,0);
				else
				{
					int tempPortValue = Sensors.getSensorValue(3, tempID);
				
					if (tempID==6)
					{
						LightSensorTotalValue+=tempPortValue;
						LightSensorCount++;
						if (LightSensorCount>9)
						{
							double tempLightValue =  (LightSensorTotalValue/10)*0.0977;
							tempLightValue = (int)(tempLightValue*10);
							tempLightValue = tempLightValue/10;
							doJSONData("3", 0, tempID, NULL, tempLightValue, false,0);
							LightSensorCount=0;
							LightSensorTotalValue=0;
						}
					} else if((tempID==5) || (tempID==7) || (tempID==1002))
					{
						if (iPort3Value != tempPortValue)
							doJSONData("3", 0, tempID, NULL, tempPortValue, false,0);
					}
					else 
						doJSONData("3", 0, tempID, NULL, tempPortValue, false,0);
						
					iPort3Value=tempPortValue;
				}
			}
			else if(_lastPort3ID>-1)
				doJSONData("3", 0, _lastPort3ID, NULL, 0, false, 2);
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
		doJSONData(tempBuffer, 0, 8, NULL, (float)intHumidity/10, false,0);
		IgnoreHeartbeatDelay=false;		// Successfully sent, now obey heartbeat

		if ((intTemperature>0) && (intTemperature<2000))
		{
			doJSONData(tempBuffer, 0,9,NULL,(float)intTemperature/10, false,0);
		}
	}
	
	if (Serial.available()>0) doReactors();
}

void NinjaObjects::sendObjects() 
{

#ifndef TESTER
	boolean IsDHT22=false;
	byte DHT22_PORT=0;

#ifdef V11
	doOnBoardAccelerometer();
#endif

#if defined(V12) || defined(VRPI10)
	do433();
#endif

// ports not present on RPi Crust
#ifndef VRPI10
	IsDHT22=doPort1(&DHT22_PORT);
#endif
	
	unsigned long currentHeartbeat = millis();

#ifndef VRPI10
	if (IsDHT22) 
		if((IgnoreHeartbeatDelay) || ((currentHeartbeat - _lastHeartbeat)>SLOW_DEVICE_HEARTBEAT)) doDHT22(DHT22_PORT);

	IsDHT22=doPort2(&DHT22_PORT);
	
	if (IsDHT22) 
		if((IgnoreHeartbeatDelay) || ((currentHeartbeat - _lastHeartbeat)>SLOW_DEVICE_HEARTBEAT)) doDHT22(DHT22_PORT);
	
	IsDHT22=doPort3(&DHT22_PORT);

	if (IsDHT22) 
		if((IgnoreHeartbeatDelay) || ((currentHeartbeat - _lastHeartbeat)>SLOW_DEVICE_HEARTBEAT)) doDHT22(DHT22_PORT);
#endif

// slow heart beat devices
	if((currentHeartbeat - _lastHeartbeat)>SLOW_DEVICE_HEARTBEAT)
	{
		doOnBoardRGB();
	
	#ifdef V11
		doOnBoardTemp();
	#endif

	_lastHeartbeat=currentHeartbeat;
	}
#else
	if (Enable433Receive)
		do433();
#endif


}
