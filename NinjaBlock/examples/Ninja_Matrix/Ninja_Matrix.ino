/*
  NinjaBlock Matrix Display Example - @askpete
  
  This sketch uses the NinjaBlock API library to write strings to two Freetronic DMD 
  displays ( http://www.freetronics.com/dmd ). It uses a forked version of the DMD library 
  that supports multiple displays amongst other things ( https://github.com/cjd/DMD ). This
  example would work just as well with anything that displays strings, e.g. an LCD
  
  Ninja Params
  ------------
  
  You need a Ninja Blocks account to use the library, use the invite code 1012BB to signup
  at https://a.ninja.is For reference the NinjaBlock params are:
  
  token    For hacking use the virtual block token from https://a.ninja.is/hacking , there
           are other ways to get a token but hard to squeeze into a library for a 328.
  nodeID   This is the boards ID. Can be any 12+char string. Its used in REST so its a
           good idea to make it easy to remember, e.g. ARDUINOBLOCK.
  vendorID Use 0 with the device IDs here http://ninjablocks.com/docs/device-ids and 
           you get pretty widgets for "free". Create your own by pinging help@ninjablocks.com.
  deviceID Identifies each device and maps to UI elements and properties that allow other apps
           to interact with your devices without knowing about them, see link above.
  guid     Think of this as port - it is used to differentiate between multiple devices of
           the same type.
     
  How to use this example
  -----------------------
  
  1) I've tested this with a Freetronics Etherten (http://www.freetronics.com/etherten ) and 
     an Arduino Ethernet (http://arduino.cc/en/Main/ArduinoBoardEthernet ), but any board or 
     shield compatible with the standard Ethernet library should be fine. Plug in the DMD 
     using the supplied breakout.

  2) Copy the NinjaBlock, DMD and TimerOne libs into your Arduino libraries dir. Collated
     here https://github.com/ninjablocks/arduino/ for your convenience.
  
  3) Cut and paste your token below from https://a.ninja.is/hacking 
  
  4) Upload and plug in the intwertubes
  
  5) Use the Ninja Dashboard, REST interface, or helper libs to write yourself messages ;)
  
  curl -H 'Content-Type: application/json' \
       -X 'PUT' \
       -d '{ "DA" : "Arduino REST FTW" }' \
       -i https://api.ninja.is/rest/v0/device/ARDUINOBLOCK_0_0_7000?user_access_token=YOURTOKEN
  
  NB: Access tokens are not the same as block tokens, get yours from https://a.ninja.is/hacking

  TODO: Write a proper how-to for this

*/

#include <SPI.h>
#include <Ethernet.h>
#include <NinjaBlock.h>
#include <DMD.h>        
#include <TimerOne.h>   
#include "Arial_black_16.h"
//#include <MemoryFree.h>

#define DISPLAYS_ACROSS 2  // be sure to set this to 1 if you only have one panel
#define DISPLAYS_DOWN 1
#define DISPLAYS_BPP 1
#define DEFAULT_VENDOR_ID 0
#define TEXT_DEVICE_ID 7000
#define WHITE 0xFF
#define BLACK 0

byte statusled = 13; // Status led on Etherten
boolean marquee = false; //Set after drawing a marquee

DMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN, DISPLAYS_BPP);

void ScanDMD()
{ 
  dmd.scanDisplayBySPI();
}

void setup(){

  pinMode(statusled, OUTPUT);  
  
  Serial.begin(9600);
  Serial.println("Starting..");
  delay(1000);   // This delay is to wait for the Ethernet Controller to get ready
  
  NinjaBlock.host = "api.ninja.is";
  NinjaBlock.port = 80;
  NinjaBlock.nodeID = "ARDUINOBLOCK";
  NinjaBlock.token = "VIRTUAL_BLOCK_TOKEN"; // Get yours from https://a.ninja.is/hacking 
  NinjaBlock.guid = "0";
  NinjaBlock.vendorID=DEFAULT_VENDOR_ID;
  NinjaBlock.deviceID=TEXT_DEVICE_ID;
  
  if (NinjaBlock.begin()==0)
    Serial.println("Init failed");

//  Serial.print("m=");
//  Serial.println(freeMemory());

  //initialize TimerOne's interrupt/CPU usage used to scan and refresh the display
  Timer1.initialize( 5000/DISPLAYS_BPP );
  Timer1.attachInterrupt( ScanDMD );

  digitalWrite(statusled, LOW);   // init the status led 

  dmd.selectFont(Arial_Black_16);
  // Clear init the DMD pixels held in RAM
  dmd.clearScreen( BLACK );
  // draw something so we know the display is working
  dmd.drawString(  6,  1, "NINJA!", 6, WHITE,BLACK );

  Serial.println("Pinging ");
  NinjaBlock.send("1");
}

void loop() {

  digitalWrite(statusled, LOW);   // turn off the status led 

  if(NinjaBlock.receive())
  {
    // If this function returns true, there are commands (data) from the server
    // Return values are:
    // NinjaBlock.strGUID
    // NinjaBlock.intVID
    // NinjaBlock.intDID
    // NinjaBlock.intDATA - if data is integer
    // NinjaBlock.strDATA - if data is string

      digitalWrite(statusled, HIGH);   // flash the status led on data received 

      Serial.print("DA:");
      if (NinjaBlock.IsDATAString) {
          Serial.print(NinjaBlock.strDATA);
          if (NinjaBlock.intDID == 7000) {
            dmd.drawMarquee(NinjaBlock.strDATA,strlen(NinjaBlock.strDATA),(32*DISPLAYS_ACROSS)-1,0, WHITE, BLACK);
            marquee = true;
          }
      } else {
        // Do something with int data
        Serial.print("INT=");
        Serial.print(NinjaBlock.intDATA);
      }

      Serial.print(" DID:");
      Serial.println(NinjaBlock.intDID);
   }

  if ( marquee ) {
    long start=millis();
    long timer=start;
    boolean ret=false;
    while(!ret){
      if ((timer+30) < millis()) {
        ret=dmd.stepMarquee(-1,0);
        timer=millis();
      }
    }
  }

}

