/*******************************************************************
* Beetle ESP32 - C3 Lightning Detector
* By John M. Wargo
* https://johnwargo.com
********************************************************************/

// https://wiki.dfrobot.com/Gravity%3A%20Lightning%20Sensor%20SKU%3A%20SEN0290

// Lightning board
// https://github.com/DFRobot/DFRobot_AS3935

#include <Wire.h>
#include "LiquidCrystal_I2C.h"
#include "DFRobot_AS3935_I2C.h"

LiquidCrystal_I2C lcd(0x20, 16, 2);  // set the LCD address to 0x20 for a 16 chars and 2 line display

#define IRQ_PIN 27

// Antenna tuning capcitance (must be integer multiple of 8, 8 - 120 pf)
#define AS3935_CAPACITANCE 96

// Indoor/outdoor mode selection
#define AS3935_INDOORS 0
#define AS3935_OUTDOORS 1
#define AS3935_MODE AS3935_INDOORS

// Enable/disable disturber detection
#define AS3935_DIST_DIS 0
#define AS3935_DIST_EN 1
#define AS3935_DIST AS3935_DIST_EN

// I2C address
#define AS3935_I2C_ADDR AS3935_ADD3

DFRobot_AS3935_I2C lightning0((uint8_t)IRQ_PIN, (uint8_t)AS3935_I2C_ADDR);

// internal variables and constants
#define lightningInterval 300000  // 5 minutes before clearing the display

String waitStr = "Waiting...";
unsigned long lastLightning;

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println();
  Serial.println("Lightning Detector");
  Serial.println("By John M. Wargo");
  Serial.println();

  Serial.println("Initializing LED Display");
  lcd.init();
  lcd.backlight();
  lcd.home();
  lcd.print("Lightning Detect");
  lcd.setCursor(0, 1);
  lcd.print("by John M. Wargo");

  Serial.println("Initializing Lightning Sensor");
  while (lightning0.begin() != 0) {
    Serial.print(".");
  }
  lightning0.defInit();

#if defined(ESP32) || defined(ESP8266)
  attachInterrupt(digitalPinToInterrupt(IRQ_PIN), lightningTrigger, RISING);
#else
  attachInterrupt(/*Interrupt No*/ 0, lightningTrigger, RISING);
#endif

  // Configure sensor
  lightning0.manualCal(AS3935_CAPACITANCE, AS3935_MODE, AS3935_DIST);
  // Enable interrupt (connect IRQ pin IRQ_PIN: 2, default)
  lcd.clear();
  lcd.home();
  lcd.print(waitStr);  
  // Initialize this to zero meaning no lightning
  lastLightning = 0;
}

void loop() {
  // have we had lightning in the last lightningInterval milliseconds?
  if (lastLightning > 0) {
    // yes, how long has it been?
    if ((millis() - lastLightning) > lightningInterval) {
      // counter expired, reset the lightning timer to zero 
      lastLightning = 0;
      // clear the display of the last lightning message
      lcd.clear();
      lcd.home();
      lcd.print(waitStr);
    }
  }
  // do a little wait here so the ESP32 has time to do housekeeping chores
  delay(100);
}

void lightningTrigger() {
  //IRQ handler for AS3935 interrupts
  uint8_t intSrc;
  uint8_t lightningDistKm;
  uint32_t lightningEnergyVal;
  String line1;  // content for display line 1
  String line2;  // content for display line 2

  Serial.println("Triggered");

  // set the last lightning time to now
  lastLightning = millis();
  // empty out line two just in case we don't need it
  line2 = "";
  // Get interrupt source
  intSrc = lightning0.getInterruptSrc();
  if (intSrc == 1) {
    // Get rid of non-distance data
    lightningDistKm = lightning0.getLightningDistKm();
    // Get lightning energy intensity
    lightningEnergyVal = lightning0.getStrikeEnergyRaw();
    line1 = "Lightning: " + String(lightningDistKm) + "km";
    line2 = "Intensity: " + String(lightningEnergyVal);
  } else if (intSrc == 2) {
    line1 = "Disturbance detected";
  } else if (intSrc == 3) {
    line1 = "Noise level too high!";
  }
  // update the console
  Serial.println(line1);
  Serial.println(line2);
  // update the display
  lcd.clear();
  lcd.home();
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}
