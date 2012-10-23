#include "NinjaBlock.h"
#include <aJson.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

EthernetClient client;
EthernetClient recvclient;
boolean isFirstConnected=true;
	
int NinjaBlockClass::begin()
{
	
	if ((host==NULL) || (nodeID==NULL) || (token==NULL) || (guid==NULL))
		return 1;		// make sure parameters are not NULL

	if(Ethernet.begin(mac)==0)
		return 1;		// Ethernet unable to obtain Dynamic IP address

	Serial.print("Device IP address: ");
	for (byte thisByte = 0; thisByte < 4; thisByte++) 
	{
		// print the value of each byte of the IP address:
		Serial.print(Ethernet.localIP()[thisByte], DEC);
		Serial.print(".");
	}
	Serial.println();
}

boolean NinjaBlockClass::decodeJSON()
{
	boolean IsJSONValid=false;
	aJsonObject* rootObject = aJson.parse(data);
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


void NinjaBlockClass::httppost(String postData)
{
	String strData;
	
	for (int i=0;i <3;i++)
	{
		if(client.connected())
		{		
			strData="POST /rest/v0/block/" + (String)nodeID + "/data HTTP/1.1\n";
			client.print(strData);
			strData = "Host: " + (String)host + ":" + port + "\n";
			client.print(strData);
			client.print("Content-Type: application/json\n");
			client.print("Accept: application/json\n");
			//client.println("Connection: close\n");
			strData="X-Ninja-Token: " + (String)token + "\n";
			client.print(strData);
			client.print("Content-Length: ");
//			strData="{\"GUID\": \"" + (String)nodeID + "_" + (String)guid + "_" + vendorID + "_" + deviceID + "\",\"G\": \"" + (String)guid + "\",\"V\": " + vendorID + ",\"D\": " + deviceID + ",\"DA\": " + data + "}";
			client.println(postData.length(), DEC);
			client.println();
			client.println(postData);	
			Serial.print("\nSent=");
			Serial.println(postData);
			return;
		}
		else
		{
			Serial.print("Not connected to server, connecting now...try ");
			Serial.println(i+1);
			client.stop();
			client.connect(host,port);
		}
	}
}

void NinjaBlockClass::send(char *data)
{
	String strSend;
	strSend="{\"GUID\": \"" + (String)nodeID + "_" + (String)guid + "_" + vendorID + "_" + deviceID + "\",\"G\": \"" + (String)guid + "\",\"V\": " + vendorID + ",\"D\": " + deviceID + ",\"DA\": \"" + (String)data + "\"}";
	httppost(strSend);
}

void NinjaBlockClass::send(int data)
{
	String strSend;
	strSend="{\"GUID\": \"" + (String)nodeID + "_" + (String)guid + "_" + vendorID + "_" + deviceID + "\",\"G\": \"" + (String)guid + "\",\"V\": " + vendorID + ",\"D\": " + deviceID + ",\"DA\": " + data + "}";
	httppost(strSend);
}

boolean NinjaBlockClass::receive(void)
{
	String strData;
	//Serial.println("receive.");
	for (int i=0;i <3;i++)
	{
		if(recvclient.connected())
		{		
			boolean gotData=false;
			int count=0;
			if(isFirstConnected)
			{
				// First time connected, so ignore HTTP header that is too long to be handled by Arduino
				while(recvclient.available())
				{
					isFirstConnected=false;
					char c = recvclient.read();
				}
			}
			else
			{
				// Ok, this should be where the real JSON command comes in
				while(recvclient.available())
				{
					char c= recvclient.read();
					if (count<DATA_SIZE)
					{
						data[count]=c;
						count++;
						if (c==0x0a) 
						{
							if (data[count-2]==0x0d)
							{
								if (data[count-3]==0x0a)
								{
									// Found ending of packet
									data[count-3]=0x0;
									data[count-2]=0x0;
									data[count-1]=0x0;
									//Parse the JSON string
									// if good gotData=true;
									if(decodeJSON()) 
									{
										gotData=true;
									}
									exit;
								}
								else
								{
									// Beginning of packet
									count=0;
								}
							}
						}
					}
				}
			}
			return gotData;
		}
		else
		{
			Serial.print("Not connected to server, connecting now...try ");
			Serial.println(i+1);
			recvclient.stop();
			if(recvclient.connect(host,port)==1)
			{
				strData="GET /rest/v0/block/" + (String)nodeID + "/commands HTTP/1.1\n";
				recvclient.print(strData);
				strData = "Host: " + (String)host + ":" + port + "\n";
				recvclient.print(strData);
				recvclient.print("Content-Type: application/json\n");
				recvclient.print("Accept: application/json\n");
				strData="X-Ninja-Token: " + (String)token + "\n";
				recvclient.print(strData);
				recvclient.println();
				isFirstConnected=true;
				Serial.print("Connected to ");
				Serial.println(host);
			}
		}
	}
}

NinjaBlockClass NinjaBlock;
