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

	Version 0.3: 25th May 2012 by JP Liew
		- restructure JSON protocol to encapsulate devices with GUID, VID, DID and DATA

	Version 0.2: 5th May 2012 by JP Liew 
		- moved to NinjaObjects library
		- added doReactors
		
	Version 0.1: by Marcus Schappi www.ninjablocks.com

*/

#include <Ninja.h>
#include <Arduino.h>
#include <aJson.h>
#include <Sensors.h>
#include <MMA8453Q.h>
#include "NinjaObjects.h"

#define recvLEN 128        // this is really bad, will need to work out dynamic length

struct __freelist {
  size_t sz;
  struct __freelist *nx;
};

uint16_t biggest;
NinjaObjects nOBJECTS;
char serInStr[recvLEN];  // array to hold the incoming serial string bytes

extern char * const __brkval;
extern struct __freelist *__flp;

char strGUID[36];
int intVID=0;
int intDID=0;
char strDATA[50];
int intDATA=0;
boolean IsDATAString=0;


NinjaObjects::NinjaObjects()
{
		
}

int getIDPinReading(int pin)
{
  analogRead(pin);
  delay(10);
  int val = analogRead(pin);
  delay(20);
  return val;
}

// given a PROGMEM string, use Serial.print() to send it out
// this is needed to save precious memory
//thanks to todbot for this http://todbot.com/blog/category/programming/
void printProgStr(const prog_char* str) {
  char c;
  if (!str) {
    return;
  }
  while ((c = pgm_read_byte(str))) {
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
  if (brkval == 0) {
    brkval = __malloc_heap_start;
  }
  cp = __malloc_heap_end;
  if (cp == 0) {
    cp = ((char *)AVR_STACK_POINTER_REG) - __malloc_margin;
  }
  if (cp <= brkval) return 0;

  freeSpace = cp - brkval;

  for (*biggest = 0, fp1 = __flp, fp2 = 0;
     fp1;
     fp2 = fp1, fp1 = fp1->nx) {
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
			//Serial.print(c);
    	delay(2);
  	}
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
					aJsonObject* jguid = aJson.getObjectItem(device,"GUID");
					aJsonObject* jvid = aJson.getObjectItem(device,"VID");
					aJsonObject* jdid = aJson.getObjectItem(device,"DID");
					aJsonObject* jdata = aJson.getObjectItem(device,"DATA");
				
					if ((jguid != NULL) && (jguid->type == aJson_String)) 
					{
						strncpy(strGUID,jguid->valuestring,36);
								
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
										strncpy(strDATA, jdata->valuestring,50);
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
			  
			Serial.println();
			// Print out decoded Params 
			Serial.print("GUID=");
			Serial.print(strGUID);
			Serial.print(",");
			
			Serial.print("VID=");
			Serial.print(intVID);
			Serial.print(",");

			Serial.print("DID=");
			Serial.print(intDID);
			Serial.print(",");

			Serial.print("DATA=");
			if (IsDATAString)
				Serial.print(strDATA);
			else
				Serial.print(intDATA);
				
			Serial.println(); 
			
			
			if(intVID==0)  // VID 0 is reserved for Ninja Blocks
			{
				switch (intDID)
				{
					case 10:		// On Board RGB Led
					{
						long colorVal = strtol(strDATA, NULL, 16);
						memset(strDATA,0, 50);
    				analogWrite(RED_LED_PIN, 255-int((colorVal&0xff0000)>>16) );
    				analogWrite(GREEN_LED_PIN, 255-int((colorVal&0x00ff00)>>8) );
    				analogWrite(BLUE_LED_PIN, 255-int((colorVal&0x0000ff)>>0) );									
						break;
					}
					
					default:
						break;		
				}
					
					
			}
				
		}
			
		else
		{
			// might need to spit out JSON error to server
			// leave this out at this stage
				
					
		}
	}	
}

void NinjaObjects::doOnBoardTemp()
{
  aJsonObject* root = aJson.createObject();
  if (root == NULL)
  {
  	Serial.println("error root"); 
    return;
  }

	if (Serial.available()>0) doReactors();
		
	aJsonObject* device = aJson.createArray();
	aJson.addItemToObject(root,"DEVICE", device);
	
	if (Serial.available()>0) doReactors();
	// On board temperature object
	aJsonObject* guid1 = aJson.createObject();
	aJson.addItemToArray(device, guid1);
	aJson.addStringToObject(guid1, "G", "0");
	aJson.addNumberToObject(guid1, "V", 0);
	aJson.addNumberToObject(guid1, "D", 1);
	aJson.addNumberToObject(guid1, "DA", Sensors.getBoardTemperature());

	if (Serial.available()>0) doReactors();
  char* string = aJson.print(root);
  if (string != NULL) 
  {
    Serial.println(string);
  } 

  aJson.deleteItem(root);
  free(string);
}

void NinjaObjects::doOnBoardAccelerometer()
{
	char tempBuffer[15];
	
  aJsonObject* root = aJson.createObject();
  if (root == NULL)
  {
  	Serial.println("error root"); 
    return;
  }

	if (Serial.available()>0) doReactors();
	aJsonObject* device = aJson.createArray();
	aJson.addItemToObject(root,"DEVICE", device);

	// On board accelerometer Object
	aJsonObject* guid2 = aJson.createObject();
	aJson.addItemToArray(device, guid2);
	aJson.addStringToObject(guid2, "G", "0");
	aJson.addNumberToObject(guid2, "V", 0);
	aJson.addNumberToObject(guid2, "D", 2);
	int x = 0, y = 0, z = 0;
	MMA.getAccXYZ(&x, &y, &z, true); //get accelerometer readings in normal mode (hi res).
	sprintf(tempBuffer, "%d,%d,%d", x,y,z);
	aJson.addStringToObject(guid2, "DA", tempBuffer);
	
	
	if (Serial.available()>0) doReactors();
		
 	char* string = aJson.print(root);
  if (string != NULL) 
  {
    Serial.println(string);
  } 

  aJson.deleteItem(root);
  free(string);
}

boolean NinjaObjects::doPort1(byte* DHT22_PORT)
{
	int tempID=0;
	char tempBuffer[15];
	boolean IsDHT22=false;
	*DHT22_PORT=0;
		
	if (Serial.available()>0) doReactors();
	// Checking Port 1
	tempID=Sensors.idTheType(getIDPinReading(ID_PIN_P1),false);
	if (tempID==8) {IsDHT22=true; *DHT22_PORT=1;}
	if (tempID>-1)
	{
  	aJsonObject* root = aJson.createObject();
  	if (root == NULL)
  	{
  		Serial.println("error root"); 
    	return false;
  	}

		aJsonObject* device = aJson.createArray();
		aJson.addItemToObject(root,"DEVICE", device);
		aJsonObject* port1 = aJson.createObject();
		aJson.addItemToArray(device, port1);
		aJson.addStringToObject(port1, "G", "1");
		aJson.addNumberToObject(port1, "V", 0);
		aJson.addNumberToObject(port1, "D", tempID);
		if (tempID==0)
			aJson.addNumberToObject(port1, "DA", 0);
		else
			aJson.addNumberToObject(port1, "DA", Sensors.getSensorValue(1, tempID));

		if (Serial.available()>0) doReactors();

	  char* string = aJson.print(root);
  	if (string != NULL) 
  	{
    	Serial.println(string);
  	} 

  	aJson.deleteItem(root);
  	free(string);

	}
	return IsDHT22;
}

boolean NinjaObjects::doPort2(byte* DHT22_PORT)
{
	int tempID=0;
	char tempBuffer[15];
	boolean IsDHT22=false;
	*DHT22_PORT=0;
			
	if (Serial.available()>0) doReactors();
	// Checking Port 2
	tempID=Sensors.idTheType(getIDPinReading(ID_PIN_P2),false);
	if (tempID==8) {IsDHT22=true; *DHT22_PORT=2;}
	if (tempID>-1)
	{
  	aJsonObject* root = aJson.createObject();
  	if (root == NULL)
  	{
  		Serial.println("error root"); 
    	return false;
  	}

		aJsonObject* device = aJson.createArray();
		aJson.addItemToObject(root,"DEVICE", device);
		aJsonObject* port2 = aJson.createObject();
		aJson.addItemToArray(device, port2);
		aJson.addStringToObject(port2, "G", "2");
		aJson.addNumberToObject(port2, "V", 0);
		aJson.addNumberToObject(port2, "D", tempID);
		if (tempID==0)
			aJson.addNumberToObject(port2, "DA", 0);
		else
			aJson.addNumberToObject(port2, "DA", Sensors.getSensorValue(2, tempID));

		if (Serial.available()>0) doReactors();

	  char* string = aJson.print(root);
  	if (string != NULL) 
  	{
    	Serial.println(string);
  	} 

  	aJson.deleteItem(root);
  	free(string);

	}
	return IsDHT22;
}

boolean NinjaObjects::doPort3(byte* DHT22_PORT)
{
	int tempID=0;
	char tempBuffer[15];
	boolean IsDHT22=false;
	*DHT22_PORT=0;	
			
	if (Serial.available()>0) doReactors();			
	// Checking Port 3
	tempID=Sensors.idTheType(getIDPinReading(ID_PIN_P3),false);
	if (tempID==8) {IsDHT22=true; *DHT22_PORT=3;}
	if (tempID>-1)
	{
  	aJsonObject* root = aJson.createObject();
  	if (root == NULL)
  	{
  		Serial.println("error root"); 
    	return false;
  	}

		aJsonObject* device = aJson.createArray();
		aJson.addItemToObject(root,"DEVICE", device);  
		aJsonObject* port3 = aJson.createObject();
		aJson.addItemToArray(device, port3);
		aJson.addStringToObject(port3, "G", "3");
		aJson.addNumberToObject(port3, "V", 0);
		aJson.addNumberToObject(port3, "D", tempID);
		if (tempID==0)
			aJson.addNumberToObject(port3, "DA", 0);
		else
			aJson.addNumberToObject(port3, "DA", Sensors.getSensorValue(3, tempID));
		
		if (Serial.available()>0) doReactors();

	  char* string = aJson.print(root);
  	if (string != NULL) 
  	{
    	Serial.println(string);
  	} 

  	aJson.deleteItem(root);
  	free(string);

	}
	return IsDHT22;
}

void NinjaObjects::doDHT22(byte port)
{
	int tempID=0;
	char tempBuffer[15];
	
 	aJsonObject* root = aJson.createObject();
 	if (root == NULL)
 	{
 		Serial.println("error root"); 
   	return;
 	}
	if (Serial.available()>0) doReactors();
	aJsonObject* device = aJson.createArray();
	aJson.addItemToObject(root,"DEVICE", device);  

	aJsonObject* DHT22 = aJson.createObject();
	aJson.addItemToArray(device, DHT22);
	sprintf(tempBuffer, "%d", port);
	aJson.addStringToObject(DHT22, "G", tempBuffer);
	aJson.addNumberToObject(DHT22, "V", 0);
	aJson.addNumberToObject(DHT22, "D", 9);		//	DID 9 is the built in temperature for DHT22
	aJson.addNumberToObject(DHT22, "DA", Sensors.getSensorValue(port, 9));	//	DID 9 is the built in temperature for DHT22
		
	if (Serial.available()>0) doReactors();
 	char* string = aJson.print(root);
 	if (string != NULL) 
 	{
   	Serial.println(string);
 	} 

 	aJson.deleteItem(root);
 	free(string);

}

void NinjaObjects::sendObjects() 
{
	boolean IsDHT22=false;
	byte DHT22_PORT=0;

	doOnBoardTemp();
	doOnBoardAccelerometer();
	
	IsDHT22=doPort1(&DHT22_PORT);
	if (IsDHT22) doDHT22(DHT22_PORT);
	IsDHT22=doPort2(&DHT22_PORT);
	if (IsDHT22) doDHT22(DHT22_PORT);
	IsDHT22=doPort3(&DHT22_PORT);
	if (IsDHT22) doDHT22(DHT22_PORT);

}
