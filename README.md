# Pulseoximeter using MAX30102

The project is primarily a port of MAX30102_by_RF (https://github.com/aromring/MAX30102_by_RF) on ESP8266 NodeMCU. MAX30102_by_RF is a significant improvement over standard Sparkfun MAX3010x library. Details of the algorithm and accuracy can be read here. (https://www.instructables.com/id/Pulse-Oximeter-With-Much-Improved-Precision/)

## Hardware

1. ESP8266 NodeMCU-cp2102
2. MH-ET Live MAX30102 (Maxim's MAX30102 replica)
3. 0.96" I2C OLED 128x64 - Blue (SSD1306)

## Connections

1. MAX30102 SCL -> OLED SCK -> NodeMCU's D1
2. MAX30102 SDA -> OLED SDA -> NodeMCU's D2
3. MAX30102 GND -> OLED GND -> NodeMCU's GND
4. MAX30102 VIN -> OLED VIN -> NodeMCU's 3V3
5. MAX30102 INT -> NodeMCU's D8

## Important

A big head-scratcher was inconsistent readings by the sensor and upon investigation, i came across this great thread: 
https://github.com/aromring/MAX30102_by_RF/issues/13#issue-601473302

MAX30102 replica breakout boards may have swapped IR and RED LEDs (mine has this issue). The work around is a single line fix included in sketch but finding it may take a whole lot of time if one isn't aware of this. So keep an eye out.  
