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

SoftwareSerial mySerial =  SoftwareSerial(5, 6);

void runBmsSensors() {
  mySerial.begin(9600);
  uint8_t data[308];
  flashStatusLED();
  if (_getBmsResponse(data) != 308) return;

  // Check the header
  if (
    data[0] != 0x55 ||
    data[1] != 0xAA ||
    data[2] != 0xEB ||
    data[3] != 0x90 ||
    data[4] != 0x02
  ) {
    return;
  }

  // Get the count of cells
  uint8_t cellCount = _getCellCount(data);
  uint16_t avgVoltage = _getCellAverage(data);
  uint16_t deviation = _getCellDeviation(data);

  uint8_t messageData[8];
  messageData[0] = 0xC0;
  messageData[1] = cellCount;
  messageData[2] = 0x02;
  messageData[3] = avgVoltage >> 8;
  messageData[4] = (uint8_t) avgVoltage;
  messageData[5] = 0x03;
  messageData[6] = deviation >> 8;
  messageData[7] = (uint8_t) deviation;

  sendSensors(messageData, 8);

  messageData[0] = 0xC7;
  messageData[1] = _getHighestCell(data) + 1;
  messageData[2] = 0xC8;
  messageData[3] = _getLowestCell(data) + 1;

  sendSensors(messageData, 4);

  // Send off the cell voltages over CAN
  _sendCellVoltages(data, cellCount);
  _sendCellWireResitance(data, cellCount);

  uint32_t overallVoltage = _getPackVoltage(data);
  messageData[0] = 0x01;
  messageData[1] = overallVoltage >> 24;
  messageData[2] = overallVoltage >> 16;
  messageData[3] = overallVoltage >> 8;
  messageData[4] = (uint8_t)overallVoltage;
  sendSensors(messageData, 5);
}

uint32_t _getBmsResponse(uint8_t* data) {
  // The trigger data to send to the BMS in order for it to send the data back
  uint8_t _jktrigger[11] =
        {0x01, 0x10, 0x16, 0x20, 0x00, 0x01, 0x02, 0x00, 0x00, 0xD6, 0xF1};

  // Prepare the MAX485 to be written to
  digitalWrite(DE, 1);
  delay(10);
  // Send the 11 bytes of data
  for (int i = 0; i < 11; i++) {
    mySerial.write(_jktrigger[i]);
    mySerial.flush();
  }
  delay(10);
  // Put the MAX485 back in RE mode
  digitalWrite(DE, 0);

  // Return the number of bytes read back
  return mySerial.readBytes(data, 308);
}

uint8_t _getCellCount(uint8_t *data) {
  // Combine the four bytes into a single uint32_t
  uint32_t bitmask = _get32BitValue(data, 70);

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

void _sendCellVoltages(uint8_t* data, uint8_t cellCount) {
  // Work out how many messages to build
  uint8_t runCount = floor(cellCount / 2);

  uint8_t voltages[6];

  for (int i = 0; i < runCount; i++) {
    // Run through both cells in this run
    for (int j = 0; j < 2; j++) {
      // Sensor address
      voltages[(j * 3)] = 0x10 + (i * 2) + j;
      // The bytes of the value
      voltages[(j * 3) + 1] = data[6 + (i * 4) + (j * 2)];
      voltages[(j * 3) + 2] = data[7 + (i * 4) + (j * 2)];
    }

    sendSensors(voltages, 6);
  }

  // Catch the instance where there is an odd number of cells
  if (cellCount % 2 > 0) {
    voltages[0] = 0x10 + runCount * 2;
    voltages[1] = data[6 + runCount * 4];
    voltages[2] = data[7 + runCount * 4];

    sendSensors(voltages, 3);
  }
}

void _sendCellWireResitance(uint8_t* data, uint8_t cellCount) {
  // Work out how many messages to build
  uint8_t runCount = floor(cellCount / 2);

  uint8_t resistances[6];

  for (int i = 0; i < runCount; i++) {
    // Run through both cells in this run
    for (int j = 0; j < 2; j++) {
      // Sensor address
      resistances[(j * 3)] = 0xD0 + (i * 2) + j;
      // The bytes of the value
      resistances[(j * 3) + 1] = data[80 + (i * 4) + (j * 2)];
      resistances[(j * 3) + 2] = data[81 + (i * 4) + (j * 2)];
    }
    // Send it off
    sendSensors(resistances, 6);
  }

  // Catch the instance where there is an odd number of cells
  if (cellCount % 2 > 0) {
    resistances[0] = 0xD0 + runCount * 2;
    resistances[1] = data[80 + runCount * 4];
    resistances[2] = data[81 + runCount * 4];
    sendSensors(resistances, 3);
  }
}

uint8_t _getHighestCell(uint8_t *data) {
  return _get8BitValue(data, 78);
}

uint8_t _getLowestCell(uint8_t *data) {
  return _get8BitValue(data, 79);
}

uint16_t _getCellDeviation(uint8_t *data) {
  return _get16BitValue(data, 74);
}

uint16_t _getCellAverage(uint8_t *data) {
  return _get16BitValue(data, 74);
}

uint32_t _getPackVoltage(uint8_t *data) {
  return _get32BitValue(data, 150);
}

int32_t _getPower(uint8_t *data) {
}
int32_t _getCurrent(uint8_t *data) {
}
uint16_t _getRemainingCapacity(uint8_t *data);
uint32_t _getNominalCapacity(uint8_t *data);
uint32_t _getCycleCount(uint8_t *data);
bool _getChargeEnabled(uint8_t *data);
bool _getDischargeEnabled(uint8_t *data);

uint16_t* _getTemperatures(uint8_t *data);

uint8_t _get8BitValue(uint8_t *data, uint16_t offset) {
  return data[offset];
}

uint16_t _get16BitValue(uint8_t *data, uint16_t offset) {
  uint16_t ret = 0;
  ret |= (uint16_t)data[offset] << 8;
  ret |= (uint16_t)data[offset + 1];
  return ret;
}

uint32_t _get32BitValue(uint8_t *data, uint16_t offset) {
  uint32_t ret = 0;

  ret |= (uint32_t)data[offset] << 24;
  ret |= (uint32_t)data[offset + 1] << 16;
  ret |= (uint32_t)data[offset + 2] << 8;
  ret |= (uint32_t)data[offset + 3];
  return ret;
}
