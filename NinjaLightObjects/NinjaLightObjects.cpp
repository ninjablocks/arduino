/*
	NinjaLightObjects.cpp - Ninja Light Objects library
	
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

#include <Arduino.h>
#include <aJSON.h>
#include <NinjaLightObjects.h>

NinjaLightObjects lOBJECTS;

NinjaLightObjects::NinjaLightObjects()
{
		intVID=0;
		intDID=0;
		intDATA=0;
		IsDATAString=false;
}

//read a string from the serial and store it in an array
int NinjaLightObjects::readSerialString () 
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

boolean NinjaLightObjects::decodeJSON()
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
boolean NinjaLightObjects::doReactors()
{
	int spos = readSerialString();

	if (spos>0)
	{
		if(decodeJSON())
		{
			if(intVID==userVID)  // VID 0 is reserved for Ninja Blocks
			{
				doJSONResponse();
				return true;	
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
	return false;
}

void NinjaLightObjects::doJSONError(int errorCode)
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

void NinjaLightObjects::doJSONResponse()
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
	aJson.addNumberToObject(guid1, "V", this->intVID);
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

void NinjaLightObjects::doJSONData(char * strGUID, int intVID, int intDID, char * strDATA, double numDATA, bool IsString, byte dataTYPE)
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
