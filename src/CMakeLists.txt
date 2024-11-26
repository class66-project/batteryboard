target_sources(batteryboard PRIVATE main.cpp)

target_link_libraries(batteryboard core spi arduino-mcp2515)

target_include_directories(batteryboard PRIVATE arduino-mcp2515)

add_library(arduino-mcp2515 STATIC
    arduino-mcp2515/mcp2515.cpp
)

target_include_directories(arduino-mcp2515 PRIVATE
    ../ArduinoCore-avr/cores/arduino
    ../ArduinoCore-avr/variants/standard
    ../ArduinoCore-avr/libraries/SPI/src
)