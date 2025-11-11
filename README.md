# OPERA_beacon_ESP32

It allows you to create a low-cost multiband OPERA TX BEACON (https://rosmodem.wordpress.com/) with an SI5351 board (8 kHz to 160 MHz with an output power of 8 dBm - 6 mW) 
and an ESP32 like the Lilygo T3_v1.6.1 OLED (https://lilygo.cc/products/lora3)

The program allows you to follow the operation of the OPERA TX using the OLED screen (if your board has one, it is very useful) 
and through the serial output if it is connected to a computer.

These programs are not necessary for the operation of OPERA beacon, only to use the OLED screen if you have it and want it:
- Adafruit_SSD1306.cpp
- Adafruit_SSD1306.h
- Fixed8x16.h

It's highly recommended to have:
- An RF receiver (e.g., ATSmini) + USB audio adapter + OPERA program to verify that the OPERA TX is transmitting and on what frequency
  
REMEMBER: that to operate a OPERA  beacon you need to have an amateur radio license. If you do not have one, it is very practical to have one and it is easier to obtain than a driver's license.

More in spanish: https://ea5jtt.blogspot.com/2025/11/opera-beacon-esp32-si5351.html



