# CMakeLists.txt for cross compiling for AVR

cmake_minimum_required(VERSION 3.23)

# GCC Toolchain
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_CXX_COMPILER avr-g++)
set(CMAKE_C_COMPILER avr-gcc)
set(CMAKE_ASM_COMPILER avr-gcc)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)


project(batteryboard)

set(PRODUCT_NAME batteryboard)

## AVR Chip configuration

# Clock speed - 16MHz
set(F_CPU 16000000UL)

# Chip type
set(MCU atmega328p)

# Default baudrate for UART
set(BAUDRATE, 9600)

# Programmer to use - Arduino as ISP
set(PROG_TYPE stk500v1)

# Fuse bits - copied from boards.txt
set(E_FUSE 0xff)
set(H_FUSE 0xde)
set(L_FUSE 0x05)
set(LOCK_BIT 0x0f)

# Tools to be used for programming AVR
set(AVR_UPLOADTOOL avrdude)

# Pass our definitions
add_definitions(
    -DF_CPU=${F_CPU}
    -DBAUD=${BAUDRATE}
)

# Linker additional parameters
set(CMAKE_EXE_LINKER_FLAGS -mmcu=${MCU})

# Compiler additional parameters
add_compile_options(
    -c
    -g
    -Os
    -w
    -std=gnu++11
    -fpermissive
    -fno-exceptions
    -ffunction-sections
    -fdata-sections
    -fno-threadsafe-statics
    -Wno-error=narrowing
    -MMD
    # -flto
    -mmcu=${MCU}
    -DF_CPU=${F_CPU}
    -DARDUINO=10607
    -DARDUINO_AVR_UNO
    -DARDUINO_ARCH_AVR
)

add_executable(batteryboard)

add_subdirectory(src)

target_include_directories(batteryboard PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ArduinoCore-avr/cores/arduino
    ArduinoCore-avr/variants/standard
    ArduinoCore-avr/libraries/**
    /usr/lib/avr/include
)

include_directories(
    ArduinoCore-avr/cores/arduino
    ArduinoCore-avr/variants/standard
    ArduinoCore-avr/libraries/SPI/src
)

add_library(core STATIC
    ArduinoCore-avr/cores/arduino/CDC.cpp
    ArduinoCore-avr/cores/arduino/USBCore.cpp
    ArduinoCore-avr/cores/arduino/HardwareSerial0.cpp
    ArduinoCore-avr/cores/arduino/IPAddress.cpp
    ArduinoCore-avr/cores/arduino/HardwareSerial2.cpp
    ArduinoCore-avr/cores/arduino/HardwareSerial3.cpp
    ArduinoCore-avr/cores/arduino/HardwareSerial1.cpp
    ArduinoCore-avr/cores/arduino/Print.cpp
    ArduinoCore-avr/cores/arduino/HardwareSerial.cpp
    ArduinoCore-avr/cores/arduino/Tone.cpp
    ArduinoCore-avr/cores/arduino/Stream.cpp
    ArduinoCore-avr/cores/arduino/PluggableUSB.cpp
    ArduinoCore-avr/cores/arduino/WInterrupts.c
    ArduinoCore-avr/cores/arduino/WMath.cpp
    ArduinoCore-avr/cores/arduino/abi.cpp
    ArduinoCore-avr/cores/arduino/hooks.c
    ArduinoCore-avr/cores/arduino/main.cpp
    ArduinoCore-avr/cores/arduino/new.cpp
    ArduinoCore-avr/cores/arduino/wiring.c
    ArduinoCore-avr/cores/arduino/WString.cpp
    ArduinoCore-avr/cores/arduino/wiring_analog.c
    ArduinoCore-avr/cores/arduino/wiring_digital.c
    ArduinoCore-avr/cores/arduino/wiring_pulse.S
    ArduinoCore-avr/cores/arduino/wiring_pulse.c
    ArduinoCore-avr/cores/arduino/wiring_shift.c
)

set_property(SOURCE ArduinoCore-avr/cores/arduino/wiring_pulse.S APPEND PROPERTY COMPILE_OPTIONS "-x" "assembler-with-cpp")


add_library(spi STATIC
    ArduinoCore-avr/libraries/SPI/src/SPI.cpp
)

# Main output
set_target_properties(batteryboard PROPERTIES OUTPUT_NAME batteryboard.elf)

# Strip binary for upload
add_custom_target(strip ALL avr-strip batteryboard.elf DEPENDS batteryboard)

# Transform binary into hex, ignoring eeprom
add_custom_target(hex ALL avr-objcopy -R .eeprom -O ihex batteryboard.elf batteryboard.hex DEPENDS strip)

# Transfor eeprom
add_custom_target(eeprom avr-objcopy -j .eeprom  --set-section-flags=.eeprom="alloc,load"  --change-section-lma .eeprom=0 -O ihex batteryboard.elf batteryboard.eep DEPENDS strip)

# Target to upload using avrdude
add_custom_target(upload avrdude -c ${PROG_TYPE} -p ${MCU} -U flash:w:batteryboard.hex DEPENDS hex)

# Target to upload eeprom with avrdude
add_custom_target(upload_eeprom avrdude -c ${PROG_TYPE} -p ${MCU}  -U eeprom:w:batteryboard.eep DEPENDS eeprom)

# Target to burn fuses
add_custom_target(fuses avrdude -c ${PROG_TYPE} -p ${MCU}  -U lfuse:w:${L_FUSE}:m -U hfuse:w:${H_FUSE}:m -U efuse:w:${E_FUSE}:m -U lock:w:${LOCK_BIT}:m )

# Make sure we clean all the extra files we are generating
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "batteryboard.hex;batteryboard.eeprom;batteryboard.lst")