#include <Ninja.h>
#include <DHT22.h>
#include <I2C.h>
#include <aJSON.h>
#include <Sensors.h>
#include <Wire.h>
#include <MMA8453Q.h>
#include <NinjaObjects.h>
#include <RCSwitch.h>

void setup()
{
  Serial.begin(9600);
  pinMode(RED_LED_PIN, OUTPUT);   
  pinMode(GREEN_LED_PIN, OUTPUT);   
  pinMode(BLUE_LED_PIN, OUTPUT);
  digitalWrite(RED_LED_PIN, HIGH);   // set the RED LED  Off
  digitalWrite(GREEN_LED_PIN, HIGH); // set the GREEN LED Off
  digitalWrite(BLUE_LED_PIN, HIGH);  // set the BLUE LED Off
}

void loop()
{
  nOBJECTS.sendObjects();      // Send Ninja Objects 
  nOBJECTS.doReactors();       // Receive Ninja Objects reactors
}
