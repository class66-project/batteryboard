/* Copyright (C) 2024 Richard Franks - All Rights Reserved
 *
 * You may use, distribute and modify this code under the
 * terms of the Apache 2.0 license.
 *
 * See LICENSE for details
 */

#pragma once

#include <Arduino.h>

#include "arduino-mcp2515/mcp2515.h"

int32_t getTempearature(uint8_t index = 0);
uint16_t get24VoltLine();

void runLocalSensors();
