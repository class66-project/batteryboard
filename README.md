# Batteryboard code

This repository contains the code for a custom ATMega328p based board that converts both local measurements
(using a current shunt, DS18B20, etc.) as well as recieved measurements from the JK BMS (using RS485) to
CAN messages that are then passed onto the rest of the system.

It utilises the MCP2515 chip for CAN communication, a MAX485 for UART<->TIA485 conversion, and an
ADS1115 for current shunt measurements.

This allows it to both relay the messages that the BMS is sending, as well as providing some independant
readings for some of the values.

## Compiling

This project uses CMake for compilation. To compile:

```
mkdir build
cd build
cmake ..
make
```

## Requirements

The following packages are required. I use a debian based WSL device to compile, so these are debian packages.
They will almost certianly be available for other distros, or will be able to be set up with Windows, but I'm
afraid you're on your own for that one!

- cmake
- gcc-avr
- avrdude
- arduino-core-avr

These can be installed in one command using

`apt install -y avrdude gcc-avr cmake arduino-core-avr`