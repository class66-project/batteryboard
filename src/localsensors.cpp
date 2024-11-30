/* Copyright (C) 2024 Richard Franks - All Rights Reserved
 *
 * You may use, distribute and modify this code under the
 * terms of the Apache 2.0 license.
 *
 * See LICENSE for details
 */

#include "localsensors.h"
#include "definitions.h"
#include "statusled.h"
#include "can.h"
#include "OneWire/OneWire.h"
#include "DallasTemperature/DallasTemperature.h"
#include "can-protocol/sensors.h"

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

bool sensors_begun = false;

int32_t getTempearature(uint8_t index) {
  if (!sensors_begun) {
    sensors.begin();
    sensors_begun = true;
  }

  sensors.requestTemperatures();

  float tempC = sensors.getTempCByIndex(0);

  return (int32_t)floor(tempC * 100);
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


void runLocalSensors() {
  struct can_frame canMsg;

  flashStatusLED();
  uint16_t _24v = get24VoltLine();
  int32_t _temperature = getTempearature();

  uint8_t data[8];

  data[0] = VOLTAGE_SENSE_1;
  data[1] = (uint8_t)_24v;
  data[2] = _24v >> 8;
  data[3] = TEMPERATURE_SENSOR_6;
  data[4] = (uint8_t)_temperature;
  data[5] = _temperature >> 8;
  data[6] = _temperature >> 16;
  data[7] = _temperature >> 24;

  sendSensors(data, 8);
}
