#include <Ninja.h>
#include <DHT22.h>
#include <I2C.h>
#include <aJSON.h>
#include <Wire.h>
#include <MMA8453Q.h>
#include <RCSwitch.h>

#include <Wire.h>
#include <LiquidCrystal.h>


// It is a known issue that conditional includes do not work via the arduino IDE.
// One must manually comment/uncomment lines as per board type defined in Ninja.h (VRPI10 or V11/V12)
//#ifdef VRPI10
#include <NinjaPi.h>
//#else //V11 or V12
//#include <Sensors.h>
//#include <NinjaObjects.h>
//#endif

LiquidCrystal lcd( 8, 9, 4, 5, 6, 7 );

static void didUpdateDisplay(Device *pDisplayDevice);
static void didUpdateDisplay(Device *pDisplayDevice) {
  Serial.print("didUpdateDisplay: "); Serial.print(pDisplayDevice->strDATA);
  Serial.println();

  lcd.clear();
  lcd.print(pDisplayDevice->strDATA);
}


//Display Device
#define DEVICES_MAX          10
Device *devices[DEVICES_MAX + 1]; //null terminated

#define DISPLAY_ROWS          2
#define DISPLAY_CHARS_PER_ROW 16

const int DISPLAY_STRING_LEN_MAX = DISPLAY_CHARS_PER_ROW * DISPLAY_ROWS;
char displayText[DISPLAY_STRING_LEN_MAX + 1]; //rcvLEN max is 128
Device displayDevice = {"0", 0, 240
    , displayText, DISPLAY_STRING_LEN_MAX, 0, true
    , &didUpdateDisplay
};
//end Display Device

void initDevices()
{
  lcd.begin(DISPLAY_CHARS_PER_ROW, DISPLAY_ROWS);

  for (int i=0; i<DEVICES_MAX + 1; i++) {
    devices[i] = (Device*)0;
  }
  strncpy(displayText, "Ninja Blocks", DISPLAY_STRING_LEN_MAX);
  devices[0] = &displayDevice;
  didUpdateDisplay(&displayDevice);
}

void initDefaultPins() {
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);
  digitalWrite(RED_LED_PIN, HIGH);            // set the RED LED  Off
  digitalWrite(GREEN_LED_PIN, HIGH);          // set the GREEN LED Off
  digitalWrite(BLUE_LED_PIN, HIGH);           // set the BLUE LED Off

#ifdef V11
  digitalWrite(BLUE_LED_PIN, LOW);           // Power on Status
#endif 
#if defined(V12) || defined(VRPI10)
  pinMode(RED_STAT_LED_PIN, OUTPUT);
  pinMode(GREEN_STAT_LED_PIN, OUTPUT);
  pinMode(BLUE_STAT_LED_PIN, OUTPUT);
  digitalWrite(RED_STAT_LED_PIN, HIGH);		// set the RED STAT LED  Off
  digitalWrite(GREEN_STAT_LED_PIN, HIGH);	        // set the GREEN STAT LED Off
  digitalWrite(BLUE_STAT_LED_PIN, LOW);	        // Power on Status
#endif
}

void setup()
{
  Serial.begin(9600);
  Serial.println();
  initDefaultPins();
  initDevices();
  nOBJECTS.connectDevices(devices);

}

void loop()
{
  nOBJECTS.sendObjects();				// Send Ninja Objects 
  nOBJECTS.doReactors();				// Receive Ninja Objects reactors  
}

