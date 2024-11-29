/* Copyright (C) 2024 Richard Franks - All Rights Reserved
 *
 * You may use, distribute and modify this code under the
 * terms of the Apache 2.0 license.
 *
 * See LICENSE for details
 */

#include <Arduino.h>
#include <SoftwareSerial.h>

#include "jkbms.h"
#include "definitions.h"
#include "statusled.h"
#include "can.h"
#include "can-protocol/sensors.h"

SoftwareSerial mySerial =  SoftwareSerial(5, 6);

JKBMS bms;

void runBmsSensors() {
  bms.run();
}

JKBMS::JKBMS() {
  this->setSerial(&mySerial);
}

void JKBMS::setSerial(SoftwareSerial *serial) {
  this->_serial = serial;
}

void JKBMS::run() {
  this->_serial->begin(9600);
  flashStatusLED();
  if (this->_getBmsResponse() != 308) return;

  if (!this->_checkHeader()) return;

  // Get the count of cells
  uint8_t cellCount = this->_getCellCount();
  uint16_t avgVoltage = this->_getCellAverage();
  uint16_t deviation = this->_getCellDeviation();

  uint8_t messageData[8];
  messageData[0] = CELL_COUNT;
  messageData[1] = cellCount;
  messageData[2] = VOLTAGE_CELL_AVERAGE;
  messageData[3] = avgVoltage >> 8;
  messageData[4] = (uint8_t) avgVoltage;
  messageData[5] = VOLTAGE_CELL_DEVIATION;
  messageData[6] = deviation >> 8;
  messageData[7] = (uint8_t) deviation;

  sendSensors(messageData, 8);

  messageData[0] = HIGHEST_CELL;
  messageData[1] = this->_getHighestCell() + 1;
  messageData[2] = LOWEST_CELL;
  messageData[3] = this->_getLowestCell() + 1;

  sendSensors(messageData, 4);

  // Send off the cell voltages over CAN
  this->_sendCellVoltages(cellCount);
  this->_sendCellWireResitance(cellCount);

  uint32_t overallVoltage = this->_getPackVoltage();
  messageData[0] = VOLTAGE_MAIN;
  messageData[1] = overallVoltage >> 24;
  messageData[2] = overallVoltage >> 16;
  messageData[3] = overallVoltage >> 8;
  messageData[4] = (uint8_t)overallVoltage;
  sendSensors(messageData, 5);

  int32_t overallPower = this->_getPower();
  messageData[0] = POWER_MAIN;
  messageData[1] = overallPower >> 24;
  messageData[2] = overallPower >> 16;
  messageData[3] = overallPower >> 8;
  messageData[4] = (uint8_t)overallPower;
  sendSensors(messageData, 5);

  int32_t overallCurrent = this->_getCurrent();
  messageData[0] = CURRENT_MAIN;
  messageData[1] = overallCurrent >> 24;
  messageData[2] = overallCurrent >> 16;
  messageData[3] = overallCurrent >> 8;
  messageData[4] = (uint8_t)overallCurrent;
  sendSensors(messageData, 5);

  uint8_t remainingCapacityPercent = this->_getRemainingCapacityPercent();
  uint16_t remainingCapacityAmpHours = this->_getRemainingCapacityAmpHours();

  messageData[0] = CAPACITY_REMAINING_PERCENT;
  messageData[1] = remainingCapacityPercent;
  messageData[2] = CAPACITY_REMAINING_AMP_HOURS;
  messageData[3] = remainingCapacityAmpHours >> 8;
  messageData[4] = (uint8_t) remainingCapacityAmpHours;
  sendSensors(messageData, 5);

  uint32_t nominalCapacity = this->_getNominalCapacity();
  messageData[0] = CAPACITY_NOMINAL;
  messageData[1] = nominalCapacity >> 24;
  messageData[2] = nominalCapacity >> 16;
  messageData[3] = nominalCapacity >> 8;
  messageData[4] = (uint8_t)nominalCapacity;
  messageData[5] = CHARGE_ENABLED;
  messageData[6] = this->_getChargeEnabled();
  sendSensors(messageData, 7);

  uint32_t cycleCount = this->_getCycleCount();
  messageData[0] = CYCLE_COUNT;
  messageData[1] = cycleCount >> 24;
  messageData[2] = cycleCount >> 16;
  messageData[3] = cycleCount >> 8;
  messageData[4] = (uint8_t)cycleCount;
  messageData[5] = DISCHARGE_ENABLED;
  messageData[6] = this->_getDischargeEnabled();
  sendSensors(messageData, 7);

  uint16_t temperatures[5];
  temperatures[0] = this->_get16BitValue(temperature1);
  temperatures[1] = this->_get16BitValue(temperature2);
  temperatures[2] = this->_get16BitValue(temperature3);
  temperatures[3] = this->_get16BitValue(tempearture4);
  temperatures[4] = this->_get16BitValue(temperature5);

  messageData[0] = TEMPERATURE_SENSOR_1;
  messageData[1] = temperatures[0] >> 8;
  messageData[2] = (uint8_t)temperatures[0];
  messageData[3] = TEMPERATURE_SENSOR_2;
  messageData[4] = temperatures[1] >> 8;
  messageData[5] = (uint8_t)temperatures[1];
  sendSensors(messageData, 6);
  messageData[0] = TEMPERATURE_SENSOR_3;
  messageData[1] = temperatures[2] >> 8;
  messageData[2] = (uint8_t)temperatures[2];
  messageData[3] = TEMPERATURE_SENSOR_4;
  messageData[4] = temperatures[3] >> 8;
  messageData[5] = (uint8_t)temperatures[3];
  sendSensors(messageData, 6);
  messageData[0] = TEMPERATURE_SENSOR_5;
  messageData[1] = temperatures[2] >> 8;
  messageData[2] = (uint8_t)temperatures[4];
  sendSensors(messageData, 3);
}

bool JKBMS::_checkHeader() {
  // Check the header
  if (
    this->_data[0] != 0x55 ||
    this->_data[1] != 0xAA ||
    this->_data[2] != 0xEB ||
    this->_data[3] != 0x90 ||
    this->_data[4] != 0x02
  ) {
    return false;
  }
  return true;
}

uint32_t JKBMS::_getBmsResponse() {
  // The trigger data to send to the BMS in order for it to send the data back
  uint8_t _jktrigger[11] =
        {0x01, 0x10, 0x16, 0x20, 0x00, 0x01, 0x02, 0x00, 0x00, 0xD6, 0xF1};

  // Prepare the MAX485 to be written to
  digitalWrite(DE, 1);
  delay(10);
  // Send the 11 bytes of data
  for (int i = 0; i < 11; i++) {
    this->_serial->write(_jktrigger[i]);
    this->_serial->flush();
  }
  delay(10);
  // Put the MAX485 back in RE mode
  digitalWrite(DE, 0);

  // Return the number of bytes read back
  return this->_serial->readBytes(this->_data, 308);
}

uint8_t JKBMS::_getCellCount() {
  // Combine the four bytes into a single uint32_t
  uint32_t bitmask = _get32BitValue(enabledCells);

  // Compare against the known values
  switch (bitmask) {
    case 0x0F000000:
    case 0x0000000F:
      return 4;
      break;
    case 0xFF000000:
    case 0x000000FF:
      return 8;
      break;
    case 0xFF0F0000:
    case 0x00000FFF:
      return 12;
      break;
    case 0xFF1F0000:
    case 0x00001FFF:
      return 13;
      break;
    case 0xFFFF0000:
    case 0x0000FFFF:
      return 16;
      break;
    case 0xFFFFFF00:
    case 0x00FFFFFF:
      return 24;
      break;
    case 0xFFFFFFFF:
      return 32;
      break;
    default:
      return 0;
  }
}

void JKBMS::_sendCellVoltages(uint8_t cellCount) {
  // Work out how many messages to build
  uint8_t runCount = floor(cellCount / 2);

  uint8_t voltages[6];

  for (int i = 0; i < runCount; i++) {
    // Run through both cells in this run
    for (int j = 0; j < 2; j++) {
      // Sensor address
      voltages[(j * 3)] = 0x10 + (i * 2) + j;
      // The bytes of the value
      voltages[(j * 3) + 1] = this->_data[cellVoltages + (i * 4) + (j * 2)];
      voltages[(j * 3) + 2] = this->_data[cellVoltages + 1 + (i * 4) + (j * 2)];
    }

    sendSensors(voltages, 6);
  }

  // Catch the instance where there is an odd number of cells
  if (cellCount % 2 > 0) {
    voltages[0] = 0x10 + runCount * 2;
    voltages[1] = this->_data[cellVoltages + runCount * 4];
    voltages[2] = this->_data[cellVoltages + 1 + runCount * 4];

    sendSensors(voltages, 3);
  }
}

void JKBMS::_sendCellWireResitance(uint8_t cellCount) {
  // Work out how many messages to build
  uint8_t runCount = floor(cellCount / 2);

  uint8_t resistances[6];

  for (int i = 0; i < runCount; i++) {
    // Run through both cells in this run
    for (int j = 0; j < 2; j++) {
      // Sensor address
      resistances[(j * 3)] = 0xD0 + (i * 2) + j;
      // The bytes of the value
      resistances[(j * 3) + 1] =
        this->_data[cellWireResistances + (i * 4) + (j * 2)];
      resistances[(j * 3) + 2] =
        this->_data[cellWireResistances + 1 + (i * 4) + (j * 2)];
    }
    // Send it off
    sendSensors(resistances, 6);
  }

  // Catch the instance where there is an odd number of cells
  if (cellCount % 2 > 0) {
    resistances[0] = 0xD0 + runCount * 2;
    resistances[1] = this->_data[cellWireResistances + runCount * 4];
    resistances[2] = this->_data[cellWireResistances + 1 + runCount * 4];
    sendSensors(resistances, 3);
  }
}

uint8_t JKBMS::_getHighestCell() {
  return this->_get8BitValue(cellHighest);
}

uint8_t JKBMS::_getLowestCell() {
  return this->_get8BitValue(cellLowest);
}

uint16_t JKBMS::_getCellDeviation() {
  return this->_get16BitValue(cellDeviation);
}

uint16_t JKBMS::_getCellAverage() {
  return this->_get16BitValue(cellAverageVoltage);
}

uint32_t JKBMS::_getPackVoltage() {
  return this->_get32BitValue(packVoltage);
}

int32_t JKBMS::_getPower() {
  return _get32BitValue(packPower);
}

int32_t JKBMS::_getCurrent() {
  return this->_get32BitValue(packCurrent);
}

uint8_t JKBMS::_getRemainingCapacityPercent() {
  return this->_get8BitValue(packCapacityRemainingPercentage);
}

uint16_t JKBMS::_getRemainingCapacityAmpHours() {
  return this->_get16BitValue(packCapacityRemainingAmpHours);
}

uint32_t JKBMS::_getNominalCapacity() {
  return this->_get32BitValue(nominalCapacity);
}

uint32_t JKBMS::_getCycleCount() {
  return this->_get32BitValue(cycleCount);
}

bool JKBMS::_getChargeEnabled() {
  uint8_t chargeEnabled = this->_get8BitValue(chargeMosfetEnabled);
  return chargeEnabled == 1;
}

bool JKBMS::_getDischargeEnabled() {
  uint8_t chargeEnabled = this->_get8BitValue(dischargeMosfetEnabled);
  return chargeEnabled == 1;
}

int8_t JKBMS::_get8BitValue(uint16_t offset) {
  return this->_data[offset];
}

int16_t JKBMS::_get16BitValue(uint16_t offset) {
  int16_t ret = 0;
  ret |= (int16_t)this->_data[offset] << 8;
  ret |= (int16_t)this->_data[offset + 1];
  return ret;
}

int32_t JKBMS::_get32BitValue(uint16_t offset) {
  int32_t ret = 0;

  ret |= (int32_t)this->_data[offset] << 24;
  ret |= (int32_t)this->_data[offset + 1] << 16;
  ret |= (int32_t)this->_data[offset + 2] << 8;
  ret |= (int32_t)this->_data[offset + 3];
  return ret;
}
