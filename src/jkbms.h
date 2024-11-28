/* Copyright (C) 2024 Richard Franks - All Rights Reserved
 *
 * You may use, distribute and modify this code under the
 * terms of the Apache 2.0 license.
 *
 * See LICENSE for details
 */

#pragma once
#include "arduino-mcp2515/mcp2515.h"



void runBmsSensors();

uint32_t _getBmsResponse(uint8_t* data);

uint8_t _getCellCount(uint8_t *data);

void _sendCellVoltages(uint8_t* data, uint8_t cellCount);
void _sendCellWireResitance(uint8_t* data, uint8_t cellCount);
uint8_t _getHighestCell(uint8_t *data);
uint8_t _getLowestCell(uint8_t *data);
uint16_t _getCellDeviation(uint8_t *data);
uint16_t _getCellAverage(uint8_t *data);

uint32_t _getPackVoltage(uint8_t *data);
int32_t _getPower(uint8_t *data);
int32_t _getCurrent(uint8_t *data);
uint16_t _getRemainingCapacity(uint8_t *data);
uint32_t _getNominalCapacity(uint8_t *data);
uint32_t _getCycleCount(uint8_t *data);
bool _getChargeEnabled(uint8_t *data);
bool _getDischargeEnabled(uint8_t *data);

uint16_t* _getTemperatures(uint8_t *data);


uint8_t _get8BitValue(uint8_t *data, uint16_t offset);
uint16_t _get16BitValue(uint8_t *data, uint16_t offset);
uint32_t _get32BitValue(uint8_t *data, uint16_t offset);
