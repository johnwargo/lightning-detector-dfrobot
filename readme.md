# DFRobot Lightning Detector

This is a **Work In Progress**.

The folks at DFRobot were kind enough to provide me with evaluation units of the [Gravity: Lightning Distance Sensor](https://www.dfrobot.com/product-1828.html) and [I2C 16x2(1602) LCD Display Module for Arduino](https://www.dfrobot.com/product-135.html). This repository wires the two together to create a lighhtning detector that displays lightning strength and distance on the display. 

The sketch's interrupt handler updates the display every time the lightning sensor detects a lightning strike, then the sketch's `loop`  function tracks a timer then clears the display if another lightning strike doesn't occur within 30000 milliseconds (5 minutes). This sketch takes a different, and I think more organized, approach to dealing with lightning strikes.

## LCD

+ https://www.dfrobot.com/product-135.html
+ [Wiki](https://wiki.dfrobot.com/I2C_TWI_LCD1602_Module__Gadgeteer_Compatible___SKU__DFR0063_)

## Lightning Sensor

+ https://www.dfrobot.com/product-1828.html
+ https://wiki.dfrobot.com/Gravity%3A%20Lightning%20Sensor%20SKU%3A%20SEN0290
