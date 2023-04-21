// modified USB_test.ino from https://github.com/tobozo/ESP32-USB-Soft-Host

////////////////////////////////////////////////////////////////////////////////////////////////
// For detailed information every 10 seconds ( USB0: Ack = 0 Nack = 0 ... ) set :
// Core Debug Level : "Debug" or "Verbose"
// Board            : "ESP32 Dev Module" ( tested with "LOLIN32 Lite" and DevKitC v4)
////////////////////////////////////////////////////////////////////////////////////////////////

//#define DEBUG_ALL                 // more details
//#define DEBUG_KBD                 // more details shown in kbdparser_de.h
//#define FORCE_TEMPLATED_NOPS

#define lang_DE                     // comment for en, uncomment for de 

#include <ESP32-USB-Soft-Host.h>
#include "usbkbd_de.h"              // KbdRptParser KeyboardReportParser
#include "kbdparser_de.h"

// ------------------------------------------------------------------------------------------ //

#if defined ARDUINO_LOLIN_S3
  #define PROFILE_NAME "LoLin S3"
  #define DP_P0  15  // always enabled
  #define DM_P0  16  // always enabled
  #define DP_P1  -1
  #define DM_P1  -1
  #define DP_P2  -1
  #define DM_P2  -1
  #define DP_P3  -1
  #define DM_P3  -1
#elif defined ARDUINO_LOLIN_D32_PRO
  // [KO] 15/13 : unassigned pins seem unresponsive
  // [OK] 22/23 : pins for MOSI/SCK work just fine
  #define PROFILE_NAME "LoLin D32 Pro"
  #define DP_P0  22  // always enabled
  #define DM_P0  21  // always enabled
  #define DP_P1  -1
  #define DM_P1  -1
  #define DP_P2  -1
  #define DM_P2  -1
  #define DP_P3  -1
  #define DM_P3  -1
#elif defined ARDUINO_M5STACK_Core2
  // [OK] 33/32 are the GROVE port pins for I2C, but there are other devices interferring on the bus
  #define PROFILE_NAME "M5Stack Core2"
  #define DP_P0  33  // always enabled
  #define DM_P0  32  // always enabled
  #define DP_P1  -1
  #define DM_P1  -1
  #define DP_P2  -1
  #define DM_P2  -1
  #define DP_P3  -1
  #define DM_P3  -1
#elif defined ARDUINO_M5STACK_FIRE
  // [KO] 16/17 : GROVE port pins for RX2/TX2 but seem unresponsive
  // [KO] 21/22 : GROVE port pins for SDA/SCL, but there are other devices interferring on the bus
  // [KO] 26/36 : GROVE port pins for I/O but seem unresponsive
  #define PROFILE_NAME "M5Stack Fire"
  #define DP_P0  16  // always enabled
  #define DM_P0  17  // always enabled
  #define DP_P1  -1
  #define DM_P1  -1
  #define DP_P2  -1
  #define DM_P2  -1
  #define DP_P3  -1
  #define DM_P3  -1
#elif defined ARDUINO_M5Stack_Core_ESP32
  // [OK] 16/17 : M5Bottom pins for RX2/TX2 work just fine
  #define PROFILE_NAME "M5Stack Gray"
  #define DP_P0  16  // always enabled
  #define DM_P0  17  // always enabled
  #define DP_P1  -1
  #define DM_P1  -1
  #define DP_P2  -1
  #define DM_P2  -1
  #define DP_P3  -1
  #define DM_P3  -1
#elif CONFIG_IDF_TARGET_ESP32C3 || defined ESP32C3
  #define PROFILE_NAME "ESP32 C3 Dev module"
  #define DP_P0   6
  #define DM_P0   8
  #define DP_P1  -1
  #define DM_P1  -1
  #define DP_P2  -1
  #define DM_P2  -1
  #define DP_P3  -1
  #define DM_P3  -1
#elif CONFIG_IDF_TARGET_ESP32S2 || defined CONFIG_IDF_TARGET_ESP32S2
  #define LOAD_TINYUSB
  #define PROFILE_NAME "ESP32 S2 Dev module"
  // [/!\] 20/19 = (USB RS/TS), might be unresponsive
  #define DP_P0  20 // ok ESP32-S2-Kaluga
  #define DM_P0  19 // ok ESP32-S2-Kaluga
  #define DP_P1  9  // ok ESP32-S2-Kaluga
  #define DM_P1  8  // ok ESP32-S2-Kaluga
  #define DP_P2  11 // ok ESP32-S2-Kaluga
  #define DM_P2  10 // ok ESP32-S2-Kaluga
  #define DP_P3  13 // ok ESP32-S2-Kaluga
  #define DM_P3  12 // ok ESP32-S2-Kaluga

#else
  // default pins tested on ESP32-Wroom
  #define PROFILE_NAME "Default Wroom"
  #define DP_P0  16 // always enabled
  #define DM_P0  17 // always enabled
  #define DP_P1  -1 // 22 // -1 to disable
  #define DM_P1  -1 // 23 // -1 to disable
  #define DP_P2  -1 // 18 // -1 to disable
  #define DM_P2  -1 // 19 // -1 to disable
  #define DP_P3  13 // -1 to disable
  #define DM_P3  15 // -1 to disable
#endif

KbdRptParser Prs;

// better use xQueueHandle usb_msg_queue in library ESP_USBSoftHost ?
static xQueueHandle cb_queue = NULL;
struct cbMessage {
  uint8_t usbNum;
  uint8_t byte_depth;
  uint8_t data[0x8];
  uint8_t data_len;
};
struct cbMessage cb_msg;

// ------------------------------------------------------------------------------------------ //
static void my_USB_DetectCB( uint8_t usbNum, void * dev )
{
  sDevDesc *device = (sDevDesc*)dev;
  printf("\nNew device detected on USB#%d\n", usbNum);
  printf("desc.bcdUSB             = 0x%04x\n", device->bcdUSB);
  //printf("desc.bDeviceClass       = 0x%02x\n", device->bDeviceClass);
  //printf("desc.bDeviceSubClass    = 0x%02x\n", device->bDeviceSubClass);
  //printf("desc.bDeviceProtocol    = 0x%02x\n", device->bDeviceProtocol);
  //printf("desc.bMaxPacketSize0    = 0x%02x\n", device->bMaxPacketSize0);
  printf("desc.idVendor           = 0x%04x\n", device->idVendor);
  printf("desc.idProduct          = 0x%04x\n", device->idProduct);
  //printf("desc.bcdDevice          = 0x%04x\n", device->bcdDevice);
  //printf("desc.iManufacturer      = 0x%02x\n", device->iManufacturer);
  //printf("desc.iProduct           = 0x%02x\n", device->iProduct);
  //printf("desc.iSerialNumber      = 0x%02x\n", device->iSerialNumber);
  //printf("desc.bNumConfigurations = 0x%02x\n", device->bNumConfigurations);
  //printf("\n");
  // if( device->iProduct == mySupportedIdProduct && device->iManufacturer == mySupportedManufacturer ) {
  //   myListenUSBPort = usbNum;
  // }
}
// ------------------------------------------------------------------------------------------ //
uint8_t parse_before = 0;
int ux_max = 0;

// for lvgl my_usb_host_read
void my_usb_host_read() {
  struct cbMessage msg;
  int num = 0;

  if (xQueueReceive(cb_queue, &msg, ( portTickType ) 10)) {

    num = uxQueueMessagesWaiting( cb_queue );
    if (ux_max < num) ux_max = num;

    if (msg.data_len == 8) {                                 // Keyboard
      //if (parse_before == 0)      // doesn't work why ?
      //  printf("\nKeyboard Test : ");  
      Prs.Parse( msg.data_len, msg.data );
      parse_before = 1;
    }  
    else if (msg.data_len == 4) {                            // Mouse
      if ( parse_before )
        printf("\n");
      parse_before = 0;

      printf("M %i/%i, ", num , ux_max);                     // message queue

      if (msg.data[1] > 0)                                   // Mouse move
        printf("MoveRight %3i ", (int8_t) msg.data[1]);  
      if (msg.data[2] > 0)
        printf("MoveDown  %3i ", (int8_t) msg.data[2]);  
      if (msg.data[3] > 0)                                   // Mouse scroll
        printf("ScrollUp  %3i ", (int8_t) msg.data[3]);  
      if (msg.data[0] > 0)                                   // Mouse button
        printf("Button    %3i ", msg.data[0] );
      printf("\n");
    }  
    else {
      printf("in: ");
      for(int k=0;k<msg.data_len;k++) {
        printf("0x%02x ", msg.data[k] );
      }
      printf("\n");
    } 
  }   
}
// ------------------------------------------------------------------------------------------ //
static void my_USB_PrintCB(uint8_t usbNum, uint8_t byte_depth, uint8_t* data, uint8_t data_len)
{
  struct cbMessage msg;

  msg.usbNum     = usbNum;
  msg.byte_depth = byte_depth;
  msg.data_len   = data_len<0x8?data_len:0x8;
  for(int k=0;k<msg.data_len;k++) {
    msg.data[k] = data[k];
  }

  xQueueSend( cb_queue, ( void * ) &msg, (TickType_t) 0 );
}
// ------------------------------------------------------------------------------------------ //
usb_pins_config_t USB_Pins_Config = { DP_P0, DM_P0, DP_P1, DM_P1, DP_P2, DM_P2, DP_P3, DM_P3 };
void setup()
{
  Serial.begin(115200);
  // waiting for the serial monitor
  delay(2000);  
  Serial.printf("\nUSB Soft Host Test for '%s'.\n", PROFILE_NAME );
  USH.init( USB_Pins_Config, my_USB_DetectCB, my_USB_PrintCB);

  cb_queue = xQueueCreate( 10, sizeof(struct cbMessage) );
}
// ------------------------------------------------------------------------------------------ //
void loop()
{
  delay(10);
  my_usb_host_read();
  //vTaskDelete(NULL);
}
// ------------------------------------------------------------------------------------------ //
