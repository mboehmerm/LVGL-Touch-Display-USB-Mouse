//#define DEBUG_ALL
//#define DEBUG_KBD
//#define FORCE_TEMPLATED_NOPS

#define lang_DE                     // keyboard : comment for en, uncomment for de 

#include <arduino.h>

#include <lvgl.h>
#include <TFT_eSPI.h>

#include <demos/lv_demos.h>
#include <examples/lv_examples.h>  //#include <../examples/lv_examples.h>

// Only for PlatformIO. PlatformIO doesn't find *.c files in subdirectories ? Arduino IDE does. 
//#define PLATFORMIO
#ifdef PLATFORMIO
  //#include <examples/widgets/btn/lv_example_btn_1.c>
  //#include <examples/widgets/msgbox/lv_example_msgbox_1.c>
  //#include <examples/widgets/menu/lv_example_menu_1.c>
  //#include <examples/widgets/tabview/lv_example_tabview_1.c>
   //#include <examples/widgets/textarea/lv_example_textarea_1.c>
   //#include <examples/widgets/win/lv_example_win_1.c>
  //#include <examples/get_started/lv_example_get_started_1.c>
  // ...
#endif

#define SCREEN_ROTATION 0                                   // set the screen rotation

/*Change to your screen resolution*/
#if (SCREEN_ROTATION == 1) || (SCREEN_ROTATION == 3)
  static const uint16_t screenWidth  = 320;                 // rotation 1 or 3
  static const uint16_t screenHeight = 240;
#else  
  static const uint16_t screenWidth  = 240;                 // rotation 0 or 2
  static const uint16_t screenHeight = 320;
#endif

TFT_eSPI tft = TFT_eSPI(screenWidth, screenHeight);         // TFT instance

#define SIZE_SCREEN_BUFFER screenWidth * screenHeight / 4   // set screen buffer size
//#define SIZE_SCREEN_BUFFER screenWidth * 10               // smaller if build error
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[ SIZE_SCREEN_BUFFER ];

#include <ESP32-USB-Soft-Host.h>
#include "usbkbd_de.h"            // class KeyboardReportParser
#include "kbdparser_de.h"         // class KbdRptParser

//#include "Cursor_11x19.c"       // mouse cursor
#include "Cursor_15x24.c"         // mouse cursor
//#include "Cursor_20x31.c"       // mouse cursor

#if defined CONFIG_IDF_TARGET_ESP32S3 || defined CONFIG_IDF_TARGET_ESP32S2 // for ESP32 S2 or S3
  #define PROFILE_NAME "ESP32 S2/S3"
  #define DP_P0  15 // USB Host Data+ Pin (must be an analog pin) (GPIO 1-20)
  #define DM_P0  16 // USB Host Data- Pin (must be an analog pin)
  #define DP_P1  18
  #define DM_P1  17
  #define DP_P2   9
  #define DM_P2   8
  #define DP_P3  -1 // 20 doesn't work
  #define DM_P3  -1 // 19 doesn't work
#else  
  #define PROFILE_NAME "Default Wroom"  // USB 2+3 disabled
  #define DP_P0  16 // always enabled
  #define DM_P0  17 // always enabled
  #define DP_P1  -1 // 22 // ( -1 to disable )
  #define DM_P1  -1 // 23 // ( -1 to disable ) used for tft
  #define DP_P2  -1 // 18 // ( -1 to disable ) used for tft
  #define DM_P2  -1 // 19 // ( -1 to disable ) used for tft
  #define DP_P3  13 // ( -1 to disable ) now available. tft CS 13 -> 14
  #define DM_P3  15 // ( -1 to disable )
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
//struct cbMessage cb_msg;

usb_pins_config_t USB_Pins_Config = { DP_P0, DM_P0, DP_P1, DM_P1, DP_P2, DM_P2, DP_P3, DM_P3 };

// ------------------------------------------------------------------------------------------ //
// Callback for ESP32-USB-Soft-Host
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
// Callback for ESP32-USB-Soft-Host
static void my_USB_PrintCB(uint8_t usbNum, uint8_t byte_depth, uint8_t* data, uint8_t data_len)
{
  struct cbMessage msg;

  msg.usbNum     = usbNum;
  msg.byte_depth = byte_depth;
  msg.data_len   = data_len<0x8?data_len:0x8;
  for(int k=0;k<msg.data_len;k++) {
    msg.data[k] = data[k];
  }

  // split here or in my_usb_host_read ?
  // if ( data_len == 8 ) 
      //xQueueSend( kbd_queue, ( void * ) &msg, (TickType_t) 0 );
  //else if ( data_len == 4 ) 
    xQueueSend( cb_queue, ( void * ) &msg, (TickType_t) 0 );
  //else 
    //; // ???
}
// ------------------------------------------------------------------------------------------ //
/* Serial debugging */
#if LV_USE_LOG != 0
void my_print(const char * buf)
{
    Serial.printf(buf);
    Serial.flush();
}
#endif
// ------------------------------------------------------------------------------------------ //
/* Display flushing */
void my_disp_flush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p )
{
    uint32_t w = ( area->x2 - area->x1 + 1 );
    uint32_t h = ( area->y2 - area->y1 + 1 );

    tft.startWrite();
    tft.setAddrWindow( area->x1, area->y1, w, h );
    tft.pushColors( ( uint16_t * )&color_p->full, w * h, true );
    tft.endWrite();

    lv_disp_flush_ready( disp );
}
// ------------------------------------------------------------------------------------------ //
// Read the keyboard
void my_kbd_read( lv_indev_drv_t * indev_driver, lv_indev_data_t * data ) { 
  uint8_t  num = 0;
  uint32_t key = 0;
  //static int ux_max_key = 0;

  data->state = LV_INDEV_STATE_REL;
  key = Prs.ReceiveKey();
  if (key) {
    //num = Prs.MessagesWaiting();
    //if (ux_max_key < num) ux_max_key = num;
    //Serial.printf("(M %i/%i, K %i)", num , ux_max_key, key);
  
    data->key   = key;
    data->state = LV_INDEV_STATE_PR;
    //Serial.print("D"); Serial.print(data->key);
  }  
  if (num > 0) data->continue_reading = true;
}  
// ------------------------------------------------------------------------------------------ //
//Read the usb_host
void my_usb_host_read( lv_indev_drv_t * indev_driver, lv_indev_data_t * data ) { 
  struct cbMessage msg;
  int num = 0;
  static byte parse_before = 0;
  static int ux_max = 0;

  data->state = LV_INDEV_STATE_REL;
  if (xQueueReceive(cb_queue, &msg, ( portTickType ) 10)) {

    num = uxQueueMessagesWaiting( cb_queue );
    if (ux_max < num) ux_max = num;

    if (msg.data_len == 8) {                                   // Keyboard
      Prs.Parse( msg.data_len, msg.data );
      parse_before = 1;
    }  
    else if (msg.data_len == 4) {                              // Mouse
      printf("M %i/%i, ", num , ux_max); 
      if (parse_before) printf("\n");
      parse_before = 0;
      if (msg.data[1] > 0) {                                   // Mouse move
        data->point.x = data->point.x + (int8_t) msg.data[1];
        if (data->point.x < 0) data->point.x = 0;
          else if (data->point.x >= screenWidth) 
            data->point.x = screenWidth - 1;
      }  
      if (msg.data[2] > 0) {
        data->point.y = data->point.y + (int8_t) msg.data[2];
        if (data->point.y < 0) data->point.y = 0;
          else if (data->point.y >= screenHeight) 
            data->point.y = screenHeight - 1;
      }  
      if ((msg.data[1] > 0) || (msg.data[2] > 0))
        printf("Mouse Move x = %i, y = %i ", data->point.x, data->point.y);  
      
      if (msg.data[3] > 0) {                                   // Mouse scroll
        printf("ScrollUp  %i ", (int8_t) msg.data[3]);  
        data->enc_diff =  (int8_t) msg.data[3];
        if ( (int8_t) msg.data[3] > 0)
          Prs.SendKey(LV_KEY_PREV);
        else if ( (int8_t) msg.data[3] < 0)
          Prs.SendKey(LV_KEY_NEXT);
      }  
      if (msg.data[0] > 0) {                                   // Mouse buttons
        printf("Button %i ", msg.data[0] );
        if (msg.data[0] == 1)                                  // left button  
          data->state = LV_INDEV_STATE_PR;  
        else if (msg.data[0] == 2)                             // right button
          Prs.SendKey(LV_KEY_ESC);
        //else if (msg.data[0] == 4)                           // middle button
      }  
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
  if (num > 0) 
    data->continue_reading = true;
}
// ------------------------------------------------------------------------------------------ //
/*Read the touchpad*/
void my_touchpad_read( lv_indev_drv_t * indev_driver, lv_indev_data_t * data )
{
   uint16_t touchX, touchY;
   bool touched = tft.getTouch( &touchX, &touchY);
   if( !touched )
   {
      data->state = LV_INDEV_STATE_REL;
   }
   else
   {
      data->state = LV_INDEV_STATE_PR;

      /*Set the coordinates*/
      #if (SCREEN_ROTATION == 1) || (SCREEN_ROTATION == 3)
        data->point.x = touchX;
        data->point.y = touchY;
      #else
        data->point.x = touchY;
        data->point.y = touchX;
      #endif

      if (data->point.x < 0) data->point.x = 0;
        else if (data->point.x >= screenWidth) data->point.x = screenWidth - 1;

      if (data->point.y < 0) data->point.y = 0;
        else if (data->point.y >= screenHeight) data->point.y = screenHeight - 1;

      //Serial.print( "Data x " );
      //Serial.println( touchX );

      //Serial.print( "Data y " );
      //Serial.println( touchY );
   }
}
// ------------------------------------------------------------------------------------------ //
//static void my_touchpad_click (struct _lv_indev_drv_t *drv, unsigned char ch)
static void my_touchpad_click (struct _lv_indev_drv_t *drv, uint8_t cx)
{
  //if (event == LV_EVENT_PRESSED) {
    //audio_play (sound_sinus2000hz_100ms);
  //}
}
// ------------------------------------------------------------------------------------------ //
// Forward declarations
void lv_example_win_2(lv_obj_t * &win2, lv_group_t * &g, lv_indev_t * &k);
void lv_example_tabview_3(lv_group_t * &g, lv_indev_t * &k);
// ------------------------------------------------------------------------------------------ //
void setup()
{
  Serial.begin(115200);

  tft.begin();        
  tft.setRotation(SCREEN_ROTATION);

    /*Set the touchscreen calibration data, the actual data for your display can be acquired using
        Generic -> Touch_calibrate example from the TFT_eSPI library */

    #if SCREEN_ROTATION == 0
      uint16_t calData[5] = { 230, 3580, 330, 3600, 4 };  //!!modified
    #elif SCREEN_ROTATION == 1
      uint16_t calData[5] = { 230, 3580, 330, 3600, 6 };
    #elif SCREEN_ROTATION == 2
      uint16_t calData[5] = { 230, 3580, 330, 3600, 2 };
    #elif SCREEN_ROTATION == 3
      uint16_t calData[5] = { 230, 3580, 330, 3600, 0 };
    #endif  

  lv_init();
  lv_disp_draw_buf_init( &draw_buf, buf, NULL, screenWidth * 10 );

  /*Initialize the display*/
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);

  /*Change the following line to your display resolution*/
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  // touchpad
  static lv_indev_drv_t touchpad_drv;
  lv_indev_drv_init( &touchpad_drv );
  touchpad_drv.type    = LV_INDEV_TYPE_POINTER;
  touchpad_drv.read_cb = my_touchpad_read;
  //touchpad_drv.feedback_cb = my_touchpad_click;
  lv_indev_drv_register( &touchpad_drv );

  // create xQueue's for usb_host and keyboard keys
  cb_queue  = xQueueCreate( 10, sizeof( struct cbMessage) );
  Prs.InitKeyQueue( 10 );

  // mouse ( and keyboard ? )
  static lv_indev_drv_t usb_host_drv;
  lv_indev_drv_init( &usb_host_drv );
  usb_host_drv.type    = LV_INDEV_TYPE_POINTER;
  usb_host_drv.read_cb = my_usb_host_read;
  lv_indev_t * mouse_indev = lv_indev_drv_register( &usb_host_drv );
 
   // mouse cursor
  lv_obj_t * mouse_cursor = lv_img_create(lv_scr_act());
  //lv_img_set_src(mouse_cursor, LV_SYMBOL_PLAY);
  //lv_img_set_src(mouse_cursor, &Cursor_11x19);
  lv_img_set_src(mouse_cursor, &Cursor_15x24);
  //lv_img_set_src(mouse_cursor, &Cursor_20x31);
  lv_indev_set_cursor(mouse_indev, mouse_cursor);

  // keyboard
  static lv_indev_drv_t kbd_drv;
  //static lv_indev_t keypad_indev;
  lv_indev_drv_init( &kbd_drv );
  kbd_drv.type    = LV_INDEV_TYPE_KEYPAD;
  kbd_drv.read_cb = my_kbd_read;
  lv_indev_t * keypad_indev = lv_indev_drv_register( &kbd_drv );
   
  // Interactive widgets - such as buttons, checkboxes, sliders, etc - can be automatically added to a default group. Just create a group with 
  lv_group_t * g = lv_group_create();                            // and set the default group with 
  lv_group_set_default(g);                                       // Don't forget to assign the input device(s) to the default group with 
  lv_indev_set_group(mouse_indev , g);
  lv_indev_set_group(keypad_indev, g); 

  // lv_group_add_obj(g, obj);

  Serial.printf("\nUSB Soft Host Test for '%s'.\n", PROFILE_NAME );
  USH.init( USB_Pins_Config, my_USB_DetectCB, my_USB_PrintCB);    // uncomment = kernel panic

    // USB Soft Host Test for 'ESP32 S2/S3'.
    // Guru Meditation Error: Core  1 panic'ed (LoadProhibited). Exception was unhandled.

    // Core  1 register dump:
    // PC      : 0x42032508  PS      : 0x00060a30  A0      : 0x82032c44  A1      : 0x3fcec1b0
    // A2      : 0x3c09151c  A3      : 0x00000000  A4      : 0x00000061  A5      : 0xffffffff  
    // A6      : 0x7ff00000  A7      : 0x7ff9d999  A8      : 0x00000011  A9      : 0xfffffdff
    // A10     : 0x0000ffff  A11     : 0x00000000  A12     : 0x00000061  A13     : 0xffffffff  
    // A14     : 0x00000000  A15     : 0x00000000  SAR     : 0x00000001  EXCCAUSE: 0x0000001c
    // EXCVADDR: 0x00000011  LBEG    : 0x40054bf0  LEND    : 0x40054c0f  LCOUNT  : 0x00000000  

  // *** uncomment only **ONE** of these lines ( examples or demos ) ***
    
  // lv_example_get_started_1();   // view examples online :
  // lv_example_btn_1();           // https://docs.lvgl.io/8.3/widgets/core/index.html or
  // lv_example_msgbox_1();        // https://docs.lvgl.io/8.3/widgets/extra/index.html 
  // lv_example_menu_1();
  // lv_example_tabview_1();
  // lv_example_textarea_1();
  // lv_example_win_1();
  // ...

  lv_demo_widgets();               // OK ( OK = enabled in lv_conf.h or platform.ini)
  // lv_demo_benchmark();          // OK weighted FPS/Max : 27/68fps (40MHz), 34/87fps(80MHz), ST7735 50/??fps
  // lv_demo_keypad_encoder();     // OK works, but I haven't an encoder
  // lv_demo_music();              // Ok ?
  // lv_demo_printer();            // MISSING
  // lv_demo_stress();             // OK
}
// ------------------------------------------------------------------------------------------ //
void loop()
{
   lv_timer_handler(); /* let the GUI do its work */
   delay( 5 );
}
// ========================================================================================== //
