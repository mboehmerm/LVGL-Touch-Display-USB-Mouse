# LVGL + Touch Display + USB-Mouse or USB-keyboard

Tested with ESP32 DevKitC v4, ili9341 display and Arduino IDE 2.1.0 / PlatformIO

Uses the library [ESP32-USB-Soft-Host](https://github.com/tobozo/ESP32-USB-Soft-Host) by tobzo

Display works with LVGL, TFT_eSPI by Bodmer or LovyanGFX

Details about display and installation can be found [here](https://github.com/mboehmerm/Touch-Display-ili9341-320x240)

![cursor_01.jpg](pictures/cursor_01.jpg)

## Connections for ESP32 DevKitC V4

Share SPI MOSI, MISO and CLK, so you need only 11 Pin's for 3 SPI devices.

| GPIO | USB   | TFT   | Touch | SD-Card | Description    |
| :--: | :---- | :---- | :---- | :------ | :------------- |
| 16   | DP_P0 |       |       |         | D+  ( green )  |
| 17   | DM_P0 |       |       |         | D-  ( white )  |
| 13   | DP_P3 |       |       |         | D+  ( green )  |
| 15   | DM_P3 |       |       |         | D-  ( white )  |
|      |       |       |       |         | 5V  ( red )    |
|      |       |       |       |         | GND ( black )  |
|------|-------|-------|-------|---------|----------------|
| 23   |       | SDI   | T_DIN | SD_MOSI | MOSI           |
| 19   |       |(SDO)  | T_DO  | SD_MISO | MISO           |
| 18   |       | SCK   | T-CLK | SD_SCK  | CLK            |
|  2   |       | DC    |       |         | DC             |
| EN   |       | RESET |       |         | RST            |
| 14   |       | CS    |       |         | CS  (TFT)      |
|  5   |       |       |       | SD_CS   | CS  (SD)       |
|  4   |       |       | T_CS  |         | CS  (Touch)    |
|      |       |       | T_IRQ |         | IRQ (not used) |
|      |       | GND   |       |         | GND            |
|      |       | VCC   |       |         | 3.3V           |
|      |       | LED   |       |         | 3.3V           |


## Installation

Details about the installation  of LVGL, TFT_eSPI, LovyanGFX and PlatformIO can be found [here](https://github.com/mboehmerm/Touch-Display-ili9341-320x240).

- [USB_Test_Scan.ino](Arduino/USB_Test_Scan/USB_Test_Scan.ino)
  - Only for testing, if there are mouses and keyboards, that work with the library "ESP32-USB-Soft-Host" i used USB_Test_Scan.ino. ( only 2 older mice work, but 3 of 4 keyboards ). Only this library must be installed or copied.
- [USB_Test_Print.ino](Arduino/USB_Test_Print/USB_Test_Print.ino)
  - More detailed informations are shown in the serial monitor.  
    - Which Mouse Button is pressed
    - Direction of mouse movement and scrolling.
    - Keyboard input is shown directy for english or german keyboard, also special characters ( @€µöäü etc. )
- [LVGL_Arduino_USB.ino](Arduino/LVGL_Arduino_USB/LVGL_Arduino_USB.ino) ( [PlatformIO version](PlatformIO/LVGL_Demo_USB) )
  - Libraries needed : LVGL, TFT_eSPI and ESP32-USB-Soft-Host.
  - Details about installation can be found [here](https://github.com/mboehmerm/Touch-Display-ili9341-320x240)  

- [LVGL_Arduino_USB_Lovyan.ino](Arduino/LVGL_Arduino_USB_Lovyan/LVGL_Arduino_USB_Lovyan.ino) ( [PlatformIO version](PlatformIO/LVGL_Demo_USB) )
  - Libraries needed : LVGL, LovyanGFX and ESP32-USB-Soft-Host
  - Details about installation can be found also [here](https://github.com/mboehmerm/Touch-Display-ili9341-320x240)  


Some files of ESP32-USB-Soft-Host are slightly modified for minimal output in the serial monitor.

# under construction !
