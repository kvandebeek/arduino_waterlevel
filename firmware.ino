#include "Ultrasonic.h" //Ultrasonic Sensor Library
#include <Wire.h> //I2C Communication Library
#include "rgb_lcd.h" // RGB LCD Library
#include <Grove_LED_Bar.h> // LED Bar Library
#include <Ciao.h> // Communication Library (allowing 

Ultrasonic UltraSonic(7);
Grove_LED_Bar LEDBar(9, 8, 0);
rgb_lcd LCD;

const float Depth_Max PROGMEM = 154.70000;
const float Depth_Max_Sensor PROGMEM = 166.00000;

//Setup of Sketch

void setup()
{
  //initialize Ciao - this allows communication with the Internet
  Ciao.begin();

  //initialize LCD screen, set Columns=16 and Rows=2
  LCD.begin(16, 2);
  LCD.setRGB(0, 0, 0);

  //initialize the LED bar
  LEDBar.begin();
}

void loop()
{

  float wL = 0.00000;
  
  for(float counter = 0; counter < 300; counter++)
  {
    wL += UltraSonic.MeasureInCentimeters();
    delay(1500);
  }

  float waterLevel = 0.00000;
  waterLevel = Depth_Max_Sensor - (wL/300);

  float waterPercent = 0.00000;
  waterPercent = waterLevel * 100 / Depth_Max;

  String Percent = "Percent: " + String(waterPercent, 1) + "%";
  String Level =   "Level  : " + String(waterLevel, 1) + "cm";

  setLCDColor(waterPercent, waterPercent);
  updateLCD(Level, Percent);
  updateLEDBar(waterPercent);  
  
  updateThingSpeak(waterLevel);
  updateDatabase(String(waterLevel),"regenwater");
}


void setLCDColor(float Red, float Green)
{
  LCD.setRGB(2.55 * (100 - int(Red)), 0 + (int(Green) * 2.55),0);
}

void updateLCD(String Level, String Percent)
{
  LCD.clear();
  LCD.noAutoscroll();
  LCD.setCursor(0, 1);
  LCD.print(Level);
  LCD.setCursor(0, 0);
  LCD.print(Percent);
}

void updateLEDBar(float waterPercent)
{
  LEDBar.setLevel((int)waterPercent / 10);
}

void updateThingSpeak(int waterLevel)

{ 

  char* CONNECTOR_TS = (char*) "rest"; //connector type
  char* SERVER_ADDR_TS = (char*) "api.thingspeak.com"; //API URL
  char* APIKEY_TS = (char*) "insertcorrectyAPIkey"; //API Key for normal data submission

  
  String uri = "/update?api_key=";
  uri += APIKEY_TS;
  uri += "&field1=";
  uri += String(waterLevel);

  CiaoData data = Ciao.write(CONNECTOR_TS, SERVER_ADDR_TS, uri);
}


void updateDatabase(String value, String sensor)
{
  char* CONNECTOR_TS = (char*) "rest"; //connector type
  char* SERVER_ADDR_DB  = (char*) "8.8.8.8"; // Change IP to the correct IP address
  char* METHOD = (char*) "POST";

  String uri = "/url.php?val=";
  uri += value;
  uri += "&sen=";
  uri += sensor;

  CiaoData data = Ciao.write(CONNECTOR_TS, SERVER_ADDR_DB, uri, METHOD);
}



