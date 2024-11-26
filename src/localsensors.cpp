/* Copyright (C) 2024 Richard Franks - All Rights Reserved
 *
 * You may use, distribute and modify this code under the
 * terms of the Apache 2.0 license.
 *
 * See LICENSE for details
 */

#include "localsensors.h"
#include "definitions.h"

uint16_t getTempearature(uint8_t index) {
    // Do nothing for now
}

/**
 * Get the 24v line voltage
 *
 * This function reads the voltage on the 24v line, returning the voltage
 * in millivolts.
 *
 * @return The value of the voltage in millivolts
 */
uint16_t get24VoltLine() {
  int rawValue = analogRead(SENSE_24V);

  uint16_t voltageInMv = round(rawValue * MILLIVOLTSPERSTEP);
  return voltageInMv;
}
