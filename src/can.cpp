/* Copyright (C) 2024 Richard Franks - All Rights Reserved
 *
 * You may use, distribute and modify this code under the
 * terms of the Apache 2.0 license.
 *
 * See LICENSE for details
 */

#include "can.h"
#include "definitions.h"
#include "statusled.h"
#include <EEPROM.h>
#include "arduino-mcp2515/can.h"
#include "arduino-mcp2515/mcp2515.h"

uint8_t stock;
uint8_t nodeId;

MCP2515 mcp2515((uint8_t)CS_PIN);

// Error flashing of if there is an error initialising the CAN
uint8_t caninit[10] = {1, 0, 1, 0, 1, 0, 0, 0, 0, 0};

void initCan() {
  nodeId = EEPROM.read(0);
  stock = EEPROM.read(1);

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

uint16_t calculateCanId(CANID_FUNCTION func) {
  uint16_t canId = 0;

  canId |= func << 8;
  canId |= stock << 6;
  canId |= nodeId;

  return canId;
}

void sendSensors(uint8_t* data, uint8_t size) {
  struct can_frame canMsg;
  canMsg.can_id = calculateCanId(SENSOR);
  canMsg.can_dlc = size;
  for (int i = 0; i < size; i++) {
    canMsg.data[i] = data[i];
  }

  flashStatusLED();
  mcp2515.sendMessage(&canMsg);
}
