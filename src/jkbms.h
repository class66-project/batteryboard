/* Copyright (C) 2024 Richard Franks - All Rights Reserved
 *
 * You may use, distribute and modify this code under the
 * terms of the Apache 2.0 license.
 *
 * See LICENSE for details
 */

#pragma once
#include <SoftwareSerial.h>
#include "arduino-mcp2515/mcp2515.h"

void runBmsSensors();

class JKBMS {
 private:
  uint8_t _data[308];
  SoftwareSerial *_serial;

  enum OFFSETS : uint16_t {
    cellVoltages = 6,
    enabledCells = 70,
    cellAverageVoltage = 74,
    cellDeviation = 76,
    cellHighest = 78,
    cellLowest = 79,
    cellWireResistances = 80,
    mosTemp = 144,
    packVoltage = 150,
    packPower = 154,
    packCurrent = 158,
    temperature1 = 162,
    temperature2 = 164,
    alarms1 = 166,
    alarms2 = 167,
    alarms3 = 168,
    balanceCurrent = 170,
    balancingAction = 172,
    packCapacityRemainingPercentage = 173,
    packCapacityRemainingAmpHours = 174,
    nominalCapacity = 178,
    cycleCount = 182,
    SOCSOH = 190,
    userAlarm = 192,
    totalRuntime = 194,
    chargeMosfetEnabled = 198,
    dischargeMosfetEnabled = 199,
    timeDcOCPR = 200,
    timeDcSCPR = 202,
    timeCOCPR = 204,
    timeCSCPR = 206,
    timeUVPR = 208,
    timeOVPR = 210,
    temperature3 = 254,
    tempearture4 = 256,
    temperature5 = 258,
  };

 public:
  JKBMS();

  void setSerial(SoftwareSerial *serial);

  void run();

 private:
  uint32_t _getBmsResponse();
  bool _checkHeader();

  uint8_t _getCellCount();

  void _sendCellVoltages(uint8_t cellCount);
  void _sendCellWireResitance(uint8_t cellCount);
  uint8_t _getHighestCell();
  uint8_t _getLowestCell();
  uint16_t _getCellDeviation();
  uint16_t _getCellAverage();

  uint32_t _getPackVoltage();
  int32_t _getPower();
  int32_t _getCurrent();
  uint8_t _getRemainingCapacityPercent();
  uint16_t _getRemainingCapacityAmpHours();
  uint32_t _getNominalCapacity();
  uint32_t _getCycleCount();
  bool _getChargeEnabled();
  bool _getDischargeEnabled();

  int8_t _get8BitValue(uint16_t offset);
  int16_t _get16BitValue(uint16_t offset);
  int32_t _get32BitValue(uint16_t offset);
};


