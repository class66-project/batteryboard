#pragma once
#include <Arduino.h>

enum CANID_FUNCTION {
  EMCY        = B001,
  SENSOR      = B010,
  SETACTUATOR = B011,
  SETCONFIG   = B100,
  GETCONFIG   = B101,
  HBEAT       = B111
};

void initCan();

uint16_t calculateCanId(CANID_FUNCTION func);

void sendSensors(uint8_t* data, uint8_t size);
