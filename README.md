# LVGL + Touch Display + USB-Mouse or USB-keyboard

# under construction !

Tested with ESP32 DevKitC v4, ili9341 display and Arduino IDE 2.1.0 ( PlatformIO )

The library ESP32-USB-Soft-Host support only USB1.1 Known working HID devices can be found [here](https://github.com/tobozo/ESP32-USB-Soft-Host) or below.

The display works with LVGL, TFT_eSPI by Bodmer or LovyanGFX. Details about this display and the installation of the libraries can be found [here](https://github.com/mboehmerm/Touch-Display-ili9341-320x240)

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
| EN   |       | RESET |       |         | RST            |
| 23   |       | SDI   | T_DIN | SD_MOSI | MOSI           |
| 19   |       |(SDO)  | T_DO  | SD_MISO | MISO           |
| 18   |       | SCK   | T-CLK | SD_SCK  | CLK            |
|  2   |       | DC    |       |         | DC             |
| 14   |       | CS    |       |         | CS  (TFT)      |
|  4   |       |       | T_CS  |         | CS  (Touch)    |
| (5)  |       |       |       | SD_CS   | CS  (SD)       |
|      |       |       | T_IRQ |         | IRQ (not used) |
|      |       | GND   |       |         | GND            |
|      |       | VCC   |       |         | 3.3V           |
|      |       | LED   |       |         | 3.3V           |



## Installation

Details about the installation of the libraries LVGL, TFT_eSPI and LovyanGFX can be found [here](https://github.com/mboehmerm/Touch-Display-ili9341-320x240).

- [USB_Test_Scan.ino](Arduino/USB_Test_Scan/USB_Test_Scan.ino)
  - Only used for testing, if there are mouses and keyboards, that work with the library "ESP32-USB-Soft-Host" i used USB_Test_Scan.ino. ( only 2 older mice work, but 3 of 4 keyboards ). Only this library must be installed or copied.
- [USB_Test_Print.ino](Arduino/USB_Test_Print/USB_Test_Print.ino)
  - More detailed informations are shown in the serial monitor.  
    - Which Mouse Button is pressed
    - Direction of mouse movement and scrolling.
    - Keyboard input is shown directy for english or german keyboard, also special characters ( @€µöäü etc. )
- [LVGL_Arduino_USB.ino](Arduino/LVGL_Arduino_USB/LVGL_Arduino_USB.ino) ( [PlatformIO version](PlatformIO/LVGL_Demo_USB) )
  - Libraries needed : LVGL, TFT_eSPI and ESP32-USB-Soft-Host.
  - Details about installation can be found [here](https://github.com/mboehmerm/Touch-Display-ili9341-320x240)  

- [LVGL_Arduino_USB_Lovyan.ino](Arduino/LVGL_Arduino_USB_Lovyan/LVGL_Arduino_USB_Lovyan.ino) ( [PlatformIO version](PlatformIO/LVGL_Demo_USB_Lovyan) )
  - Libraries needed : LVGL, LovyanGFX and ESP32-USB-Soft-Host
  - Details about installation can be found also [here](https://github.com/mboehmerm/Touch-Display-ili9341-320x240)  

**Problems with PlatformIO programs :**
- All PlatformIO programs :
  - many warnings : unsigned conversion from 'int' to 'uint8_t'
  - build_flags = -Wno-narrowingw necessary
- USB_Test_Scan ok
- USB_Test_Print ok
- LVGL_Demo_USB : ok
- LVGL_Demo_USB_Lovyan : ok if you don't hit any keyboard button


Some files of ESP32-USB-Soft-Host are slightly modified for minimal output in the serial monitor.

# working HID devices

| Type     | bcdUSB | idVendor |idProduct| Description  |
| :------- | :----: | :-----   | :------ | :----------  |
| Mouse    | 0x0200 | 0x04f2   | 0x0923  | HP MSU0923   |
| Mouse    | 0x0110 | 0x062a   | 0x0000  | Speedlink Snappy BL-6141 |
| Keyboard | 0x0110 | 0x04ca   | 0x0050  | Acer SK-9621 |
| Keyboard | 0x0200 | 0x04f2   | 0x1516  | HP KU-1516   |
| Keyboard | 0x0110 | 0x03f0   | 0x1441  | HP SK-2029   |

None of my wireless HID devices work. Also the keyboard HP KBAR211.

No connection with logic level shifter TXS0108E (YF08E).
