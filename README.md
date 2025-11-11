# OPERA_beacon_ESP32

It allows you to create a low-cost multiband OPERA TX BEACON (https://rosmodem.wordpress.com/) with an SI5351 board (8 kHz to 160 MHz with an output power of 8 dBm - 6 mW) 
and an ESP32 like the Lilygo T3_v1.6.1 OLED (https://lilygo.cc/products/lora3)

Mode  Time    Carrier frecuency (RX USB = Cf - 1,5 kHz)
05    0,128s  144,181; 432,0915 y 1296,6015  MHz
1     0,256s  21.075,5; 24.926,5; 28.071,5; 50.701,5 y 70.094,5 kHz
2     0,512s  3.548,5; 5.290,5; 7.039,5; 10.136,5 y 14.063,5 kHz
4     1,024s  1.837,5 kHz
8     2,048s  477 kHz
32    8,192s  136 kHz
65   16,384s
2H   32,768s

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



