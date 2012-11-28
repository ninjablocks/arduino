#include "NinjaBlock.h"
// #include <MemoryFree.h> 

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

EthernetClient client;
EthernetClient recvclient;

int NinjaBlockClass::begin()
{
	
	if ((host==NULL) || (nodeID==NULL) || (token==NULL) || (guid==NULL))
		return 1;		// make sure parameters are not NULL

	if(Ethernet.begin(mac)==0)
		return 1;		// Ethernet unable to obtain Dynamic IP address

	Serial.print("IP: ");
	for (byte thisByte = 0; thisByte < 4; thisByte++) 
	{
		// print the value of each byte of the IP address:
		Serial.print(Ethernet.localIP()[thisByte], DEC);
		Serial.print(".");
	}
	Serial.println();
}

void NinjaBlockClass::httppost(char *postData)
{
	//String strData;
	char strData[DATA_SIZE];
	
	for (int i=0;i <3;i++)
	{
		if(client.connected())
		{		
			sendHeaders(true,client);
			client.print("Content-Length: ");
			client.println(strlen(postData));
			client.println();
			client.println(postData);	
			Serial.print("Sent=");
			Serial.println(postData);
			
			// client.flush();
			while(client.available())
			{
				// httppost will return an HTTP response, if a device needs to process the return,
				// this is the place code should be added to do the processing
				// at the moment, we just clear all the buffer to prevent http read buffer being overflow
				client.flush();
			}
			return;
		}
		else
		{
			Serial.print("_");
			client.stop();
			client.connect(host,port);
		}
	}
}

void NinjaBlockClass::sendHeaders(boolean isPOST, EthernetClient hclient) {
	char strData[DATA_LEN];
	if (isPOST)  
		strcpy(strData,"POST");
	else 
		strcpy(strData,"GET");
	strcat(strData," /rest/v0/block/");
	strcat(strData, nodeID);
	if (isPOST)  
		strcat(strData,"/data");
	else 
		strcat(strData, "/commands");
	strcat(strData, " HTTP/1.1\r\n");
	hclient.print(strData);
	strcpy(strData,"Host: "); 
	strcat(strData ,host);
	strcat(strData, "\r\n");
	hclient.print(strData);
	hclient.print("User-Agent: Ninja Arduino 1.1\r\n");
	hclient.print("Content-Type: application/json\r\n");
	hclient.print("Accept: application/json\r\n");
	strcpy(strData,"X-Ninja-Token: ");
	strcat(strData, token);
	strcat(strData,"\r\n");
	hclient.print(strData);
}

void NinjaBlockClass::send(char *data)
{
	ninjaMessage(false, 0, data);
}

void NinjaBlockClass::send(int data)
{
	ninjaMessage(true, data, 0);
}

void NinjaBlockClass::ninjaMessage(boolean isInt, int intData, char *charData) {
	char strSend[DATA_SIZE];
	char strNumber[6];
	
	strcpy(strSend,"{\"GUID\": \"");
	strcat(strSend,nodeID);
	strcat(strSend, "_" );
	strcat(strSend,guid);
	strcat(strSend, "_");
	itoa(vendorID, strNumber, 10);
	strcat(strSend, strNumber);
	strcat(strSend, "_");
	itoa(deviceID, strNumber, 10);
	strcat(strSend, strNumber);
	strcat(strSend, "\",\"G\": \"");
	strcat(strSend, guid);
	strcat(strSend, "\",\"V\": ");
	itoa(vendorID, strNumber, 10);
	strcat(strSend, strNumber);
	strcat(strSend,",\"D\": ");
	itoa(deviceID, strNumber, 10);
	strcat(strSend, strNumber);
	strcat(strSend, ",\"DA\": ");
	if (isInt) {
		itoa(intData, strNumber, 10);
		strcat(strSend, strNumber);
	} else {
		strcat(strSend, "\"");
		strcat(strSend, charData);
		strcat(strSend, "\"");
	}
	strcat(strSend, "}");
	httppost(strSend);
}

boolean NinjaBlockClass::receive(void)
{
	for (int i=0;i <2;i++)
	{
		if(recvclient.connected())
		{	
			boolean gotData=false;
			int count=0;
			int d = 0;
			int start;

			while(recvclient.available())
			{
				char c = recvclient.read();
				// Uncomment the below to print the entire response to the console
				// Serial.print(c);

				if (d != 4) {

					// This is a very coarse way to find the end of the headers
					// We are looking for \r\n\r\n 
					if (c == 0x0a || c == 0x0d) {
						d++;
					} else {
						d = 0;
					}

				} else {

					if (count<DATA_SIZE)
					{
						data[count]=c;

						// 0x22 " 
						// 0x2c , 
						// 0x3a : 
						// 0x41 A
						// 0x44 D
						// 0x47 G
						
						// We aren't using aJSON to save a few hundred bytes.
						// JSON separator if prev 2 chars ":
						if (data[count-2]==0x22 && data[count-1]==0x3a) {
							// If " we assume the value is a string
							if (c == 0x22) {
								c = recvclient.read();							
								if (data[count-3]==0x47) {
									start = count;
									while (c != 0x22) {
										data[count]=c;
										c = recvclient.read();	
										count++;
									}
									memcpy( strGUID, &data[start], count-start );
									strGUID[count-start] = 0;
								}
								else if (data[count-4]==0x44 && data[count-3]==0x41) {
									start = count;
									while (c != 0x22) {
										data[count]=c;
										c = recvclient.read();	
										count++;
									}
									memcpy( strDATA, &data[start], count-start );
									strDATA[count-start] = 0;
									IsDATAString=true;
									gotData=true;
								}
							} else {
								// Assume Int value 
								if (data[count-3]==0x44) {
									start = count;
									while (c != 0x2c) {
										data[count]=c;
										c = recvclient.read();
										count++;
									}
									// There must be a cleaner way to do this
									char tmp[count-start];
									memcpy( tmp, &data[start], count-start );
									tmp[count-start] = 0;
									intDID = atoi(tmp);
								} 
								else if (data[count-3]==0x56) {
									start = count;
									while (c != 0x2c) {
										data[count]=c;
										c = recvclient.read();
										count++;
									}
									// There must be a cleaner way to do this
									char tmp[count-start];
									memcpy( tmp, &data[start], count-start );
									tmp[count-start] = 0;
									intVID = atoi(tmp);
								}

							}
						}

						count++;

						// DEBUG
						// Look for \n as the end of the message
						// if (c==0x0a) 
						// {
						// 	Serial.print("strDATA=");
						// 	Serial.print(strDATA);
						// 	Serial.print(" strGUID=");
						// 	Serial.print(strGUID);
						// 	Serial.print(" intDID=");
						// 	Serial.print(intDID);
						// 	Serial.print(" intVID=");
						// 	Serial.println(intVID);
						// }
					}
				}
			}
			if (d == 4) {
				recvclient.flush();
				delay(100);
				recvclient.stop();
				delay(100);
			}
			return gotData;
		}
		else
		{
			Serial.print(".");
			recvclient.stop();
			// Create connection
			if(recvclient.connect(host,port)==1)
			{
				sendHeaders(false, recvclient);
				recvclient.println();
			}
		}
	}
}

NinjaBlockClass NinjaBlock;
