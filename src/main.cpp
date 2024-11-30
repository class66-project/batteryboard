/* Copyright (C) 2024 Richard Franks - All Rights Reserved
 *
 * You may use, distribute and modify this code under the
 * terms of the Apache 2.0 license.
 *
 * See LICENSE for details
 */
#include <Arduino.h>
#include <SPI.h>  // Library for using SPI Communication
#include <EEPROM.h>

#include "arduino-mcp2515/mcp2515.h"  // Library for using CAN Communication
#include "can.h"
#include "definitions.h"
#include "localsensors.h"
#include "jkbms.h"

/**
 * Main setup function
 *
 * Sets up the board ready for the main loop to run
 */
void setup() {
  EEPROM.put(0, B100);
  EEPROM.put(1, B00);
  Serial.begin(115200);
  SPI.begin();   // Begins SPI communication

  initCan();

  pinMode(STATUS_LED, OUTPUT);
  pinMode(SENSE_24V, INPUT);
  pinMode(DE, OUTPUT);
  digitalWrite(DE, 0);
}


/**
 * Main loop function
 *
 * This loop is run continuously
 */
void loop() {
  runLocalSensors();
  runBmsSensors();

  delay(500);
}
