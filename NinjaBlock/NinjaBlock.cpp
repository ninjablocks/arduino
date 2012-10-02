#include "NinjaBlock.h"

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

/*
	char *nodeID;
	char *token;
	char *guid;
	uint16_t vendorID=0;
	uint16_t deviceID=0;
*/

EthernetClient client;

int NinjaBlockClass::begin()
{
	if ((host==NULL) || (nodeID==NULL) || (token==NULL) || (guid==NULL))
		return 1;		// make sure parameters are not NULL

	if(Ethernet.begin(mac)==0)
		return 1;		// Ethernet unable to obtain Dynamic IP address

	client.connect(host, port);	
	
}

void NinjaBlockClass::send(int data)
{
	String strData;
	
	if(!client.connected())
	{
		client.stop();
		client.connect(host,port);
	}

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
	strData="{\"GUID\": \"" + (String)nodeID + "_" + (String)guid + "_" + vendorID + "_" + deviceID + "\",\"G\": \"" + (String)guid + "\",\"V\": " + vendorID + ",\"D\": " + deviceID + ",\"DA\": " + data + "}";
	client.println(strData.length(), DEC);
	client.println();
	client.println(strData);	
	Serial.print("\nSent=");
	Serial.println(strData);
}

void NinjaBlockClass::send(char *data){
	
}

NinjaBlockClass NinjaBlock;
