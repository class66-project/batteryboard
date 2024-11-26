add_executable(batteryboard)

file(GLOB SRC_FILES "*.cpp") 

target_sources(batteryboard PRIVATE ${SRC_FILES})

target_link_libraries(batteryboard core SPI arduino-mcp2515)

target_include_directories(batteryboard PRIVATE
    arduino-mcp2515
    ../ArduinoCore-avr/cores/arduino
    ../ArduinoCore-avr/variants/standard
    ../ArduinoCore-avr/libraries/SPI/src
)

add_library(arduino-mcp2515 STATIC
    arduino-mcp2515/mcp2515.cpp
)

target_include_directories(arduino-mcp2515 PRIVATE
    ../ArduinoCore-avr/cores/arduino
    ../ArduinoCore-avr/variants/standard
    ../ArduinoCore-avr/libraries/SPI/src
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