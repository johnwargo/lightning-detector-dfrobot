/*******************************************************************
* Beetle ESP32 - C3 Lightning Detector
* By John M. Wargo
* https://johnwargo.com
********************************************************************/

// https://wiki.dfrobot.com/Gravity%3A%20Lightning%20Sensor%20SKU%3A%20SEN0290

// Lightning board
// https://github.com/DFRobot/DFRobot_AS3935

#include "DFRobot_AS3935_I2C.h"

#include <Wire.h>
#include "LiquidCrystal_I2C.h"
LiquidCrystal_I2C lcd(0x20, 16, 2);  // set the LCD address to 0x20 for a 16 chars and 2 line display

volatile int8_t AS3935IsrTrig = 0;

#if defined(ESP32) || defined(ESP8266)
#define IRQ_PIN 0
#else
#define IRQ_PIN 2
#endif

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

void AS3935_ISR();

DFRobot_AS3935_I2C lightning0((uint8_t)IRQ_PIN, (uint8_t)AS3935_I2C_ADDR);

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
  attachInterrupt(digitalPinToInterrupt(IRQ_PIN), AS3935_ISR, RISING);
#else
  attachInterrupt(/*Interrupt No*/ 0, AS3935_ISR, RISING);
#endif

  // Configure sensor
  lightning0.manualCal(AS3935_CAPACITANCE, AS3935_MODE, AS3935_DIST);
  // Enable interrupt (connect IRQ pin IRQ_PIN: 2, default)

  lcd.clear();
  lcd.home();
  lcd.print("Init complete");
}

void loop() {
  // It does nothing until an interrupt is detected on the IRQ pin.
  while (AS3935IsrTrig == 0) { delay(1); }
  delay(5);

  // Reset interrupt flag
  AS3935IsrTrig = 0;

  // Get interrupt source
  uint8_t intSrc = lightning0.getInterruptSrc();
  if (intSrc == 1) {
    // Get rid of non-distance data
    uint8_t lightningDistKm = lightning0.getLightningDistKm();
    Serial.println("Lightning detected");
    Serial.print("Distance: ");
    Serial.print(lightningDistKm);
    Serial.println(" km");

    // Get lightning energy intensity
    uint32_t lightningEnergyVal = lightning0.getStrikeEnergyRaw();
    Serial.print("Intensity: ");
    Serial.print(lightningEnergyVal);
    Serial.println("");
  } else if (intSrc == 2) {
    Serial.println("Disturbance detected");
  } else if (intSrc == 3) {
    Serial.println("Noise level too high!");
  }
}

//IRQ handler for AS3935 interrupts
void AS3935_ISR() {
  AS3935IsrTrig = 1;
}
