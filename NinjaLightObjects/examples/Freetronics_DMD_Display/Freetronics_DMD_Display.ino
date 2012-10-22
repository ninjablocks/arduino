#include <SPI.h>        //SPI.h must be included as DMD is written by SPI (the IDE complains otherwise)
#include <DMD.h>        //
#include <TimerOne.h>   //
#include "SystemFont5x7.h"
#include "Arial_black_16.h"
#include <NinjaLightObjects.h>
#include <aJSON.h>

#define DISPLAYS_ACROSS 1
#define DISPLAYS_DOWN 1
#define USER_VENDOR_ID 0
#define USER_DEVICE_ID 7000

DMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN);

void ScanDMD()
{ 
  dmd.scanDisplayBySPI();
}

void setup(void)
{
   Serial.begin(9600);
   
   lOBJECTS.userVID=USER_VENDOR_ID;  // Let the library know that this is your VENDOR ID
   
   //initialize TimerOne's interrupt/CPU usage used to scan and refresh the display
   Timer1.initialize( 5000 );           //period in microseconds to call ScanDMD. Anything longer than 5000 (5ms) and you can see flicker.
   Timer1.attachInterrupt( ScanDMD );   //attach the Timer1 interrupt to ScanDMD which goes to dmd.scanDisplayBySPI()
   //clear/init the DMD pixels held in RAM
  dmd.clearScreen( true );   //true is normal (all pixels off), false is negative (all pixels on)
  dmd.clearScreen(true);
  dmd.selectFont(Arial_Black_16);
  dmd.drawMarquee("NinjaBlocks Demo",16,(32*DISPLAYS_ACROSS)-1,0);
}

void loop(void)
{
  long start=millis();
  long timer=start;
  boolean ret=false;
  while(!ret)
  {
     if ((timer+30) < millis()) 
     {
       ret=dmd.stepMarquee(-1,0);
       timer=millis();
     }
     if(lOBJECTS.doReactors())    // Check if there is any commands from Host for your USER_VENDOR_ID
     {
       if (lOBJECTS.intDID==USER_DEVICE_ID)  // is the data for USER_DEVICE_ID ?
       {
         dmd.clearScreen(true);
         dmd.drawMarquee(lOBJECTS.strDATA,strlen(lOBJECTS.strDATA),(32*DISPLAYS_ACROSS)-1,0);
       }
     }
   }
   // Ok display finished 1 round of Marquee, let server know
   lOBJECTS.doJSONData("0", USER_VENDOR_ID, USER_DEVICE_ID, "DONE", 0, true, 0);

}
