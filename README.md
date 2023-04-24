# LVGL + Touch Display + USB-Mouse + USB-Keyboard


Tested with ESP32 DevKitC v4 ( or Lolin ESP32 lite ), ili9341 display and Arduino IDE 2.1.0 ( PlatformIO )

The library ESP32-USB-Soft-Host 0.1.4 supports only USB1.1 and known working HID devices can be found [here](https://github.com/tobozo/ESP32-USB-Soft-Host) or below.

The display works with LVGL, TFT_eSPI by Bodmer or LovyanGFX. Details about this display and the installation of the libraries can be found [here](https://github.com/mboehmerm/Touch-Display-ili9341-320x240). Remember that here the CS pin has changed from GPIO15 to GPIO14, because GPIO15 is needed as second USB device.

![cursor_01.jpg](pictures/cursor_01.jpg)

## Connections for ESP32 DevKitC V4

Share SPI - MOSI, MISO and CLK, so you need only 11 Pin's for 3 SPI devices.

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
| 23   |       | SDI   | T_DIN |(SD_MOSI)| MOSI           |
| 19   |       |(SDO)  | T_DO  |(SD_MISO)| MISO           |
| 18   |       | SCK   | T-CLK |(SD_SCK) | CLK            |
|  2   |       | DC    |       |         | DC             |
|**14**|       | CS    |       |         | CS  (TFT)      |
|  4   |       |       | T_CS  |         | CS  (Touch)    |
| (5)  |       |       |       | (SD_CS) | CS  (SD)       |
|      |       |       | T_IRQ |         | IRQ (not used) |
|      |       | GND   |       |         | GND            |
|      |       | VCC   |       |         | 3.3V           |
|      |       | LED   |       |         | 3.3V           |

D+ and D- are directly connected to the GPIO's without using any resistor. SD-card and GPIO 5 are not used in this test.

## Test programs

Details about installation and configuration of the libraries LVGL, TFT_eSPI and LovyanGFX can be found [here](https://github.com/mboehmerm/Touch-Display-ili9341-320x240). All modified files can be found above in the folder [Arduino](Arduino/) or [PlatformIO](PlatformIO/). Version 0.1.4 of ESP32-USB-Soft-Host is used.

- [USB_Test_Scan.ino](Arduino/USB_Test_Scan) ( [PlatformIO version](PlatformIO/USB_Test_Scan) )
  - Used for testing, whether there are mice and keyboards, that work with "ESP32-USB-Soft-Host". Only this library must be installed or copied. The line "#define DEBUG_ALL" is not commented, so a lot of informations are shown. For further informations every 10 seconds or so ( USB0: Ack = 0 Nack = 0 ... ) set the "Core Debug Level" to "Debug" or "Verbose" in the Arduino IDE.
- [USB_Test_Input.ino](Arduino/USB_Test_Input) ( [PlatformIO version](PlatformIO/USB_Test_Input) ).
  - More/other information are shown in the serial monitor.  
    - Which Mouse Button is pressed
    - Direction of mouse movement and scrolling.
    - Keyboard input is shown directy for english ( //#define lang_DE ) or german keyboard ( #define lang_DE ), also special characters ( @€µöäü etc. )
- [LVGL_Arduino_USB.ino](Arduino/LVGL_Arduino_USB) ( [PlatformIO version](PlatformIO/LVGL_Demo_USB) )
  - Libraries needed : LVGL , TFT_eSPI and ESP32-USB-Soft-Host 0.1.4.
  - Mouse and mouse button(s) are working fine, mouse cursor too.
  - Mouse position x/y and buttons are shown in the serial monitor.
  - Keyboard keys are shown in the serial monitor but not used in LVGL (to be done).

- [LVGL_Arduino_USB_Lovyan.ino](Arduino/LVGL_Arduino_USB_Lovyan) ( [PlatformIO version](PlatformIO/LVGL_Demo_USB_Lovyan) )
  - Version for the library LovyanGFX

The target of these programs is to find out how HID mice and keyboards can be used with an ESP32.

## Problems with the PlatformIO versions :
Compiling errors and warnings are related to the special characters ( öäüß€ etc.) in the library "usbkbd_de.h". 

To avoid the **[-Wnarrowing]** errors the "platformio.ini" should contain the build flag "-Wno-narrowing" :
```
; ...
lib_deps = 
  ESP32-USB-Soft-Host
build_flags =
  -Wno-narrowing      ; disable [-Wnarrowing] errors
  ;-Wno-multichar     ; disable [-Wmultichar] warnings
; ...
```

# Library files ESP32-USB-Soft-Host

Some files of ESP32-USB-Soft-Host are slightly modified for minimal output of messages in the serial monitor, if "DEBUG_ALL" is commented in the line "//#define DEBUG_ALL".

The library files i [downloaded](https://github.com/tobozo/ESP32-USB-Soft-Host) and used for testing are recognized by VSCode/PlatformIO as "ESP32-USB-Soft-Host @ 0.1.4".


# My working HID devices

| Type     | bcdUSB | idVendor |idProduct| Description  |
| :------- | :----: | :-----   | :------ | :----------  |
| Mouse    | 0x0200 | 0x04f2   | 0x0923  | HP MSU0923   |
| Mouse    | 0x0110 | 0x062a   | 0x0000  | Speedlink Snappy BL-6141 |
| Keyboard | 0x0110 | 0x04ca   | 0x0050  | Acer SK-9621 |
| Keyboard | 0x0200 | 0x04f2   | 0x1516  | HP KU-1516   |
| Keyboard | 0x0110 | 0x03f0   | 0x1441  | HP SK-2029   |

None of my wireless HID devices work. Neither the keyboard HP KBAR211.

No device was found when i use the logic level shifter TXS0108E (YF08E). 
