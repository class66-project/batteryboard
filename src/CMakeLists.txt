target_sources(batteryboard PRIVATE main.cpp)

target_link_libraries(batteryboard core spi arduino-mcp2515)