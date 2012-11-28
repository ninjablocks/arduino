/*
 *  NinjaBlock ninjablocks.com API wrapper
 *  Author: JP Liew, Pete Moore @askpete
 *  Source: git://github.com/ninjablocks/arduino
 *  This library provides a basic interface to the Ninja Blocks API.
 *  Use it to send sensor data and/or listen for commands. See exampes
 *  for usage or http://help.ninjablocks.com and search for Arduino library.
 */
 
#ifndef ninjablock_h
#define ninjablock_h

#include <SPI.h>
#include <Ethernet.h>

#define DATA_SIZE  128
#define GUID_LEN	36
#define DATA_LEN	64

class NinjaBlockClass {

public:
	char *host;
	char *nodeID;
	char *token;
	char *guid;
	char data[DATA_SIZE];
	int port;
	int vendorID;
	int deviceID;

	char strGUID[GUID_LEN];
	int intVID;
	int intDID;
	int intDATA;
	char strDATA[DATA_LEN];
	boolean IsDATAString;
	
	int begin();
	void send(int data);
	void send(char *data);
	boolean receive(void);
	void httppost(char *postData);
	boolean decodeJSON();

private:
	void ninjaMessage(boolean, int intData, char *charData);
	void sendHeaders(boolean isPOST, EthernetClient hclient);
};

extern NinjaBlockClass NinjaBlock; 

#endif
