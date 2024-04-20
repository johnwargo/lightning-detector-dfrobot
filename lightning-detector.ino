/*******************************************************************
* Beetle ESP32 - C3 Lightning Detector
* By John M. Wargo
* https://johnwargo.com
********************************************************************/
// References:
// https://wiki.dfrobot.com/Gravity%3A%20Lightning%20Sensor%20SKU%3A%20SEN0290
// https://github.com/DFRobot/DFRobot_AS3935
// https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/intr_alloc.html
// https://lastminuteengineers.com/handling-esp32-gpio-interrupts-tutorial/

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
// #define lightningInterval 300000  // 5 minutes before clearing the display
#define lightningInterval 60000  // 1 minute before clearing the display

String waitStr = "Waiting...";
String dashes = "======================";

volatile bool IRQ_EVENT = false;
unsigned long lastLightning;
int counter = 0;
int counterLimit = 50;

// https://lastminuteengineers.com/handling-esp32-gpio-interrupts-tutorial/
void IRAM_ATTR isr() {
  IRQ_EVENT = true;
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println();
  Serial.println(dashes);
  Serial.println("| Lightning Detector |");
  Serial.println("| By John M. Wargo   |");
  Serial.println(dashes);

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
  // if (lightning0.defInit() != 0) Serial.println("Reset failed");

#if defined(ESP32) || defined(ESP8266)
  attachInterrupt(digitalPinToInterrupt(IRQ_PIN), isr, RISING);
#else
  attachInterrupt(/*Interrupt No*/ 0, isr, RISING);
#endif

  // Configure sensor
  lightning0.manualCal(AS3935_CAPACITANCE, AS3935_MODE, AS3935_DIST);
  // Enable interrupt (connect IRQ pin IRQ_PIN: 2, default)
  lcd.clear();
  lcd.home();
  lcd.print(waitStr);
  // Initialize this to zero meaning no lightning
  lastLightning = 0;

  // esp_intr_dump();
}

void loop() {
  if (IRQ_EVENT) {
    IRQ_EVENT = false;
    Serial.println("\nTriggered");
    updateDisplay();
  }
  checkDisplay();

  // Serial.print(".");
  // counter += 1;
  // if (counter > counterLimit) {
  //   counter = 0;
  //   Serial.println();
  // }
  // do a little wait here so the ESP32 has time to do housekeeping chores
  delay(100);
}

void updateDisplay() {
  uint8_t intSrc;
  uint8_t lightningDistKm;
  uint32_t lightningEnergyVal;
  String line1;  // content for display line 1
  String line2;  // content for display line 2

  // set the last lightning time to now
  lastLightning = millis();
  // empty out the text line
  line2 = "<empty>";
  // Get interrupt source
  intSrc = lightning0.getInterruptSrc();
  // Serial.println(String(intSrc));
  switch (intSrc) {
    case 1:
      lightningDistKm = lightning0.getLightningDistKm();
      line1 = "Lightning: " + String(lightningDistKm) + "km";
      lightningEnergyVal = lightning0.getStrikeEnergyRaw();
      line2 = "Intensity: " + String(lightningEnergyVal);
      break;
    case 2:
      line1 = "Disturbance detected";
      break;
    case 3:
      line1 = "Noise level too high!";
      break;
    default:
      line1 = "<UNKNOWN>";
      break;
  }

  // lightning0.printAllRegs();

  // update the console
  Serial.println(line1);
  Serial.println(line2);
  // update the display
  lcd.clear();
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
  lcd.home();
}

void checkDisplay() {
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
}
