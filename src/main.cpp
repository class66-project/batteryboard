/* Copyright (C) 2024 Richard Franks - All Rights Reserved
 *
 * You may use, distribute and modify this code under the
 * terms of the Apache 2.0 license.
 *
 * See LICENSE for details
 */

#include <SPI.h>  // Library for using SPI Communication
#include <mcp2515.h>  // Library for using CAN Communication

#include "definitions.h"
#include "localsensors.h"

// A single CAN frame, re-used to send can messages
struct can_frame canMsg;

// Main MCP2515 object, used for communicating over CAN
MCP2515 mcp2515(CS_PIN);

// Error flashing of if there is an error initialising the CAN
uint8_t caninit[10] = {1, 0, 1, 0, 1, 0, 0, 0, 0, 0};

/**
 * Function to display an error on the status LED.
 *
 * This function takes in an error code (defined as a uint_8t[10])
 * and then displays it non-stop.
 *
 * @param error a uint_8t[10] pointer of the pattern to display
 */ 
void error_led(uint8_t *error) {
  int i;
  while (1) {
    for (i=0; i< 10; i++) {
      digitalWrite(STATUS_LED, caninit[i]);
      delay(100);
    }
  }
}

/**
 * Main setup function
 *
 * Sets up the board ready for the main loop to run
 */
void setup() {
  Serial.begin(115200);
  SPI.begin();   // Begins SPI communication

  pinMode(STATUS_LED, OUTPUT);
  pinMode(SENSE_24V, INPUT);

  // Initialise thet CAN tranciever, catching any errors
  bool error = false;
  if (mcp2515.reset() != mcp2515.ERROR_OK) {
    error = true;
  }
  if (mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ) != mcp2515.ERROR_OK) {
    error = true;
  }
  if (mcp2515.setNormalMode() != mcp2515.ERROR_OK) {
    error = true;
  }
  if (error) {
    // If we get an error, then display it on the error LED
    error_led(caninit);
  }
}


/**
 * Main loop function
 *
 * This loop is run continuously
 */
void loop() {
  digitalWrite(STATUS_LED, 1);
  delay(100);
  digitalWrite(STATUS_LED, 0);

  uint16_t _24v = get24VoltLine();
  Serial.println(_24v);
  canMsg.can_id = 0x104;
  canMsg.can_dlc = sizeof(uint16_t) + 1;
  canMsg.data[0] = 0x04;
  for (int i=0; i < sizeof(uint16_t); i++) {
    canMsg.data[i + 1] = _24v >> i*8;
  }
  mcp2515.sendMessage(&canMsg);

  delay(1000);
}
