/* Copyright (C) 2024 Richard Franks - All Rights Reserved
 *
 * You may use, distribute and modify this code under the
 * terms of the Apache 2.0 license.
 *
 * See LICENSE for details
 */

#pragma once

#include <Arduino.h>
#include "can-protocol/enums.h"

void initCan();

uint16_t calculateCanId(CANID_FUNCTION func);

void sendSensors(uint8_t* data, uint8_t size);
