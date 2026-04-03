#include "Ultrasonic.h"
#include <Wire.h>
#include "rgb_lcd.h"
#include <Grove_LED_Bar.h>
#include <Ciao.h>

// Hardware instances
Ultrasonic UltraSonic(7);
Grove_LED_Bar LEDBar(9, 8, 0);
rgb_lcd LCD;

// Configuration constants
const float DEPTH_MAX = 154.70;
const float DEPTH_MAX_SENSOR = 166.00;
const int SAMPLES = 10;
const int SAMPLE_DELAY_MS = 1500;
const float RGB_SCALE = 2.55;
const byte LED_LEVELS = 10;
const char* CONNECTOR_TYPE = "rest";
const char* THINGSPEAK_SERVER = "api.thingspeak.com";
const char* DATABASE_SERVER = "8.8.8.8";
const char* DATABASE_METHOD = "POST";
const char* THINGSPEAK_API_KEY = "insertcorrectyAPIkey";
const char* SENSOR_NAME = "regenwater";

void setup() {
  Ciao.begin();
  LCD.begin(16, 2);
  LCD.setRGB(0, 0, 0);
  LEDBar.begin();
}

void loop() {
  // Measure water level
  float waterLevel = measureWaterLevel();
  float waterPercent = calculatePercentage(waterLevel);

  // Update displays
  updateDisplay(waterLevel, waterPercent);
  updateThingSpeak(waterLevel);
  updateDatabase(waterLevel);
  
  delay(5000); // Wait before next measurement cycle
}

/**
 * Measure water level by averaging multiple ultrasonic readings
 * @return Water level in centimeters
 */
float measureWaterLevel() {
  float totalDistance = 0.0;
  
  for (int i = 0; i < SAMPLES; i++) {
    totalDistance += UltraSonic.MeasureInCentimeters();
    delay(SAMPLE_DELAY_MS);
  }
  
  float averageDistance = totalDistance / SAMPLES;
  return DEPTH_MAX_SENSOR - averageDistance;
}

/**
 * Calculate water level as percentage of maximum depth
 * @param waterLevel Water level in centimeters
 * @return Percentage (0-100)
 */
float calculatePercentage(float waterLevel) {
  float percent = (waterLevel * 100) / DEPTH_MAX;
  return constrain(percent, 0.0, 100.0);
}

/**
 * Update all display outputs (LCD, LED bar, colors)
 */
void updateDisplay(float waterLevel, float waterPercent) {
  updateLCDDisplay(waterLevel, waterPercent);
  updateLEDBar(waterPercent);
  setLCDColor(waterPercent);
}

/**
 * Set LCD backlight color based on water level percentage
 * Green (full) -> Red (empty)
 * @param percent Water level percentage
 */
void setLCDColor(float percent) {
  byte red = (byte)(RGB_SCALE * (100 - percent));
  byte green = (byte)(RGB_SCALE * percent);
  LCD.setRGB(red, green, 0);
}

/**
 * Display water level and percentage on LCD
 */
void updateLCDDisplay(float waterLevel, float waterPercent) {
  LCD.clear();
  LCD.noAutoscroll();
  
  LCD.setCursor(0, 0);
  LCD.print("Level: ");
  LCD.print(waterLevel, 1);
  LCD.print("cm");
  
  LCD.setCursor(0, 1);
  LCD.print("Percent: ");
  LCD.print(waterPercent, 1);
  LCD.print("%\n");
}

/**
 * Update LED bar to show water level
 * @param waterPercent Water level as percentage
 */
void updateLEDBar(float waterPercent) {
  byte level = (byte)(waterPercent / (100.0 / LED_LEVELS));
  LEDBar.setLevel(constrain(level, 0, LED_LEVELS));
}

/**
 * Send water level to ThingSpeak
 * @param waterLevel Water level in centimeters
 */
void updateThingSpeak(float waterLevel) {
  String uri = "/update?api_key=";
  uri += THINGSPEAK_API_KEY;
  uri += "&field1=";
  uri += String(waterLevel, 1);

  Ciao.write(CONNECTOR_TYPE, THINGSPEAK_SERVER, uri);
}

/**
 * Send water level to local database
 * @param waterLevel Water level in centimeters
 */
void updateDatabase(float waterLevel) {
  String uri = "/url.php?val=";
  uri += String(waterLevel, 1);
  uri += "&sen=";
  uri += SENSOR_NAME;

  Ciao.write(CONNECTOR_TYPE, DATABASE_SERVER, uri, DATABASE_METHOD);
}