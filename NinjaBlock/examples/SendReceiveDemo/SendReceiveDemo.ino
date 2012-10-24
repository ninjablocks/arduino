/*
  NinjaBlock Emulator library using Arduino Etherten - by JP Liew 24th Oct 2012
  
  This code shows how a user could use NinjaBlock's Arduino library to create their own NinjaBlock 
  using an Arduino + Ethernet Shield to emulate a NinjaBlock thus creating their own custom devices.
  
  In this example, we create two sensor types (type 5 - button and type 11 - 433mhz receiver) and 1 type of actuator.
  
  In order to initialise the NinjaBlock library, user need to get the following from Ninja Block - email help@ninjablocks.com
  
  1. A token.
  2. host - Ninja Cloud host name.  Ask for a non SSL version.
  3. port - Ninja Cloud host port. Ask for a non SSL version.
  4. vendorID - User Vendor ID ( you could use Vendor ID 0 just to muck around first, if you apply for your own Vendor ID, you get to define and create your own device ID.
  5. deviceID - ID of a device (you could use Ninja's predefined ID, or apply for a vendor ID to create your own device)
  6. nodeID - This is the string serial number of your product, for example if you have an Arduino board, make up an ID for this board)
  7. guid - This is the unique ID of the sensor/actuator connected to your Arduino board, for example if you have two button with device ID of 5, assigning guid 1 and 2 respectively
     allows them to be differentiated when they show up on the user interface.
     
  How to use this example
  -----------------------
  1) Once all the above parameters have been collected, change these values in the code.
  
  2) A Freetronics Etherten or an Arduino with ethernet shield is required.  Digital Pin 3 and 5 are used as a sensor trigger,
  connect them to ground to trigger a command send to Ninja Cloud.  (For a better result, please implement a debounce for these pins)
  
  3) Upload sketch and enjoy.
*/

#include <SPI.h>
#include <Ethernet.h>
#include <NinjaBlock.h>
#include <aJSON.h>

#define buttonpin3    3
#define buttonpin5    5

void setup(){
  
  pinMode(buttonpin3, INPUT);
  digitalWrite(buttonpin3, HIGH);
  pinMode(buttonpin5, INPUT);
  digitalWrite(buttonpin5, HIGH);
  
  Serial.begin(9600);
  Serial.println("Starting..");
  delay(1000);   // This delay is to wait for the Ethernet Controller to get ready, documented in Arduino Ethernet Lib
  
  NinjaBlock.host = "10.0.1.39";
  NinjaBlock.port = 3000;
  NinjaBlock.nodeID = "MS1111111111";
  NinjaBlock.token = "d8c3ffc3-0273-4537-ba55-dfac2cbe3834";
  NinjaBlock.guid = "1";
  NinjaBlock.vendorID=0;
  NinjaBlock.deviceID=1002;

  if (NinjaBlock.begin()==0)
    Serial.println("Unable to init NinjaBlock\n");
    
}

void loop() {

  if(NinjaBlock.receive())    
  {
    // if function return true, there are data/command from the server
    // Values return are:
    // NinjaBlock.strGUID
    // NinjaBlock.intVID
    // NinjaBlock.intDID
    // NinjaBlock.intDATA - if data is integer
    // NinjaBlock.strDATA - if data is string
    
      Serial.print("Received ");
      if (NinjaBlock.IsDATAString)
        Serial.print(NinjaBlock.strDATA);
      else
        Serial.print(NinjaBlock.intDATA);
        
      Serial.print(" for device ");
      Serial.println(NinjaBlock.intDID);
      
      // Do something for the device here
      // example digitalWrite(4, HIGH);
   }

  if (digitalRead(buttonpin3)==0)
  {
    NinjaBlock.deviceID=5;
    NinjaBlock.send(0);
    while(digitalRead(buttonpin3)==0);
    NinjaBlock.deviceID=5;
    NinjaBlock.send(1);
  }

  if (digitalRead(buttonpin5)==0)
  {
    while(digitalRead(buttonpin5)==0);
    NinjaBlock.deviceID=11;
    NinjaBlock.send("10101010");
  }
}

