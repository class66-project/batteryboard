/* Copyright (C) 2024 Richard Franks - All Rights Reserved
 *
 * You may use, distribute and modify this code under the
 * terms of the Apache 2.0 license.
 *
 * See LICENSE for details
 */

#pragma once

// Chip select pin used for MCP2515
#define CS_PIN 10
// Baud rate of the CANBUS
#define CAN_BAUDRATE 500e3

// Millivolts per step in the ADC.
// This value should be calibrated on a per-board basis, as it
// will depend on the resistors
#define MILLIVOLTSPERSTEP 29.3

// Status LED output
#define STATUS_LED 4

// 24v input sense pin
#define SENSE_24V A0

// DE/RE pin for MAX485
#define DE 9

// pin for DS18B20
#define ONE_WIRE_BUS 8
