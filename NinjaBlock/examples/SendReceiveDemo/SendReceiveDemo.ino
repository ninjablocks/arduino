#include <SPI.h>
#include <Ethernet.h>
#include <NinjaBlock.h>
#include <aJSON.h>

#define buttonpin3    3
#define buttonpin5    5

#define led 13

void setup(){
  
  pinMode(buttonpin3, INPUT);
  digitalWrite(buttonpin3, HIGH);
  pinMode(buttonpin5, INPUT);
  digitalWrite(buttonpin5, HIGH);
  
  pinMode(led,OUTPUT);
  digitalWrite(led,HIGH);
  
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
      //Serial.println(NinjaBlock.data); 
      Serial.print("Received ");
      if (NinjaBlock.IsDATAString)
        Serial.print(NinjaBlock.strDATA);
      else
        Serial.print(NinjaBlock.intDATA);
        
      Serial.print(" for device ");
      Serial.println(NinjaBlock.intDID);
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

