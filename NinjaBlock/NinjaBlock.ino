#include "JsonSerial.h"
#include "NinjaPacket.h"
#include "PortManager.h"
#include "NinjaLED.h"
#include "OnBoardManager.h"
#include "Heartbeat.h"
#include "RFPacket.h"

#include "CommonProtocolEncoder.h"
#include "CommonProtocolDecoder.h"

OnBoardManager  onBoardManager;
PortManager     portManager;
NinjaLED        leds;
NinjaPacket     ninjaPacket;

int freeRam ()
{
  extern int __heap_start, *__brkval; 
  int v; 

  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

void setup()
{
  delay(2000);
  
  jsonSerial.setup(9600);
  leds.setup();
  onBoardManager.setup();
}

void loop()
{
  // 1. Check for serial data
  if(jsonSerial.read(&ninjaPacket))
  {
    if(ninjaPacket.getGuid() > 0)
      portManager.handle(&ninjaPacket);
    else
      onBoardManager.handle(&ninjaPacket);

    /*Serial.print("G=");
    Serial.println(ninjaPacket.getGuid());
    Serial.print("D=");
    Serial.println(ninjaPacket.getDevice());
    Serial.print("DA=");
    Serial.println(ninjaPacket.getData());
    Serial.print("Ram=");
    Serial.println(freeRam());*/
  }
  
  heartbeat.check();
  
  // 2. Check hardware ports for changes
  portManager.check();
  
  // 3. Check onboard components for incoming data
  onBoardManager.check();

  heartbeat.resume();
}

