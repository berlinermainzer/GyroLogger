Gyro Logger V2

Ressources
http://www.esp8266learning.com/wemos-mini-adxl345-example.php
https://steve.fi/hardware/d1-flash/
https://techtutorialsx.com/2019/02/24/esp32-arduino-removing-a-file-from-the-spiffs-file-system/
https://github.com/Chris--A/PrintEx#printex-library-for-arduino- - for float sprintf

ADXL345
D1 - SCL
D2 - SDA

MAX8
RX - DIN,----
D4 - LRCLK
D8 - BCLK



AudioOutputI2S::SetPinout(int bclk, int wclk, int dout)
SetPinout(26, 25, 22)
D8, D4, Rx	

    .bck_io_num = 26,
    .ws_io_num = 25,
    .data_out_num = 22,
	
	[ESP32 IO26 – I2S codec BCK]
[ESP32 IO22 – I2S codec DATA]
[ESP32 IO25 – I2S codec LRCK]
