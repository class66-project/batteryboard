/* Copyright (C) 2024 Richard Franks - All Rights Reserved
 *
 * You may use, distribute and modify this code under the
 * terms of the Apache 2.0 license.
 *
 * See LICENSE for details
 */

#include <SPI.h>  // Library for using SPI Communication
#include <mcp2515.h>  // Library for using CAN Communication

#define CS_PIN 10
#define CAN_BAUDRATE 500000

// This should be calibrated using a reliable voltage source
#define MILLIVOLTSPERSTEP 2.93


#define STATUS_LED 4
#define SENSE_24V A0
#define SENSE_5V A1


struct can_frame canMsg;
MCP2515 mcp2515(10);  // SPI CS Pin 10

uint8_t caninit[10] = {1, 0, 1, 0, 1, 0, 0, 0, 0, 0};

void error_led(uint8_t *error) {
  int i;
  while (1) {
    for (i=0; i< 10; i++) {
      digitalWrite(STATUS_LED, caninit[i]);
      delay(100);
    }
  }
}

void setup() {
  Serial.begin(115200);
  SPI.begin();   // Begins SPI communication

  pinMode(STATUS_LED, OUTPUT);
  pinMode(SENSE_24V, INPUT);
  pinMode(SENSE_5V, INPUT);

  mcp2515.reset();
  // Sets CAN at speed 500KBPS and Clock 8MHz
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();  // Sets CAN at normal mode
}

uint16_t get24VoltLine() {
  int rawValue = analogRead(SENSE_24V);

  uint32_t voltageInMv = round(rawValue * MILLIVOLTSPERSTEP);
  return voltageInMv;
}

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
