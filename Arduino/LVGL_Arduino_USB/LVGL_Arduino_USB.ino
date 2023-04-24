//#define DEBUG_ALL
//#define DEBUG_KBD
//#define FORCE_TEMPLATED_NOPS

//#define lang_DE                     // kbd : comment for en, uncomment for de 

#include <arduino.h>

#include <lvgl.h>
#include <TFT_eSPI.h>
#include <demos/lv_demos.h>

#define SCREEN_ROTATION 1                                   // set the screen rotation

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

  #define PROFILE_NAME "Default Wroom"  // USB 1+2 disabled
  #define DP_P0  16 // always enabled
  #define DM_P0  17 // always enabled
  #define DP_P1  -1 // 22 // ( -1 to disable )
  #define DM_P1  -1 // 23 // ( -1 to disable ) used for tft
  #define DP_P2  -1 // 18 // ( -1 to disable ) used for tft
  #define DM_P2  -1 // 19 // ( -1 to disable ) used for tft
  #define DP_P3  13 // ( -1 to disable ) now available. tft CS 13 -> 14
  #define DM_P3  15 // ( -1 to disable )

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

usb_pins_config_t USB_Pins_Config = { DP_P0, DM_P0, DP_P1, DM_P1, DP_P2, DM_P2, DP_P3, DM_P3 };

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
int ux_max = 0;
byte parse_before = 0;

/*Read the usb_host*/
void my_usb_host_read( lv_indev_drv_t * indev_driver, lv_indev_data_t * data ) { 
  struct cbMessage msg;
  int num = 0;

  data->state = LV_INDEV_STATE_REL;
  if (xQueueReceive(cb_queue, &msg, ( portTickType ) 10)) {

    num = uxQueueMessagesWaiting( cb_queue );
    if (ux_max < num) ux_max = num;
 
    if (msg.data_len == 8) {                                   // Keyboard
      //printf("M %i, ", num ); 
      Prs.Parse( msg.data_len, msg.data );  // to be done
      parse_before = 1;
    }  
    else if (msg.data_len == 4) {                              // Mouse
      printf("M %i/%i, ", num , ux_max); 
      if (parse_before) printf("\n");
      parse_before = 0;
      if (msg.data[1] > 0) {                                   // Mouse move
        data->point.x = data->point.x + (int8_t) msg.data[1];
        if (data->point.x < 0) data->point.x = 0;
        else if (data->point.x >= screenWidth) data->point.x = screenWidth - 1;
        //printf("MoveHor   %i, x = %i ", (int8_t) msg.data[1], data->point.x);  
        //printf("MoveRight %i ", (int8_t) msg.data[1]);         // signed int !
      }  
      if (msg.data[2] > 0) {
        data->point.y = data->point.y + (int8_t) msg.data[2];
        if (data->point.y < 0) data->point.y = 0;
        else if (data->point.y >= screenHeight) data->point.y = screenHeight - 1;
        //printf("MoveVert  %i, y = %i ", (int8_t) msg.data[2], data->point.y);  
        //printf("MoveDown  %i ", (int8_t) msg.data[2]);  
      }  
      if ((msg.data[1] > 0) || (msg.data[2] > 0))
        printf("Mouse Move x = %i, y = %i ", data->point.x, data->point.y);  
      
      if (msg.data[3] > 0) {                                   // Mouse scroll
        printf("ScrollUp  %i ", (int8_t) msg.data[3]);  
      }  
      if (msg.data[0] > 0) {                                   // Mouse button
        printf("Button %i ", msg.data[0] );
        if (msg.data[0] == 1) {
          data->state = LV_INDEV_STATE_PR;
          //printf("LV_INDEV_STATE_PR");  
        }  
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
void my_touchpad_read( lv_indev_drv_t * indev_driver, lv_indev_data_t * data )  //// modified !!!
{
    uint16_t touchX, touchY;

    //bool touched = touch.Pressed();//tft.getTouch( &touchX, &touchY, 600 );
    bool touched = tft.getTouch( &touchX, &touchY, 600 );

    if( !touched )
    {
        data->state = LV_INDEV_STATE_REL;
    }
    else
    {   
        //touchX = touch.X();
        //touchY = touch.Y();
        
        data->state = LV_INDEV_STATE_PR;

        /*Set the coordinates*/
        #if (SCREEN_ROTATION == 1) || (SCREEN_ROTATION == 3)
          data->point.x = touchX;
          data->point.y = touchY;
        #else
          data->point.x = touchY;
          data->point.y = touchX;
        #endif
        
        // problems with calibration ?
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
void setup()
{
    Serial.begin( 115200 ); /* prepare for possible serial debug */

    String LVGL_Arduino = "Hy Arduino! ";
    LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();

    Serial.println( LVGL_Arduino );
    Serial.println( "I am LVGL_Arduino" );

    lv_init();

#if LV_USE_LOG != 0
    lv_log_register_print_cb( my_print ); /* register print function for debugging */
#endif

    tft.begin();
    tft.setRotation( SCREEN_ROTATION );

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
    lv_disp_drv_init( &disp_drv );

    /*Change the following line to your display resolution*/
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register( &disp_drv );

    // touchpad
    static lv_indev_drv_t touchpad_drv;
    lv_indev_drv_init( &touchpad_drv );
    touchpad_drv.type = LV_INDEV_TYPE_POINTER;
    touchpad_drv.read_cb = my_touchpad_read;
    //touchpad_drv.feedback_cb = my_touchpad_click;
    lv_indev_drv_register( &touchpad_drv );

    // mouse ( and keyboard ? )
    static lv_indev_drv_t usb_host_drv;
    static lv_indev_t u;
    lv_indev_drv_init( &usb_host_drv );
    usb_host_drv.type = LV_INDEV_TYPE_POINTER;
    usb_host_drv.read_cb = my_usb_host_read;
    lv_indev_t * mouse_indev = lv_indev_drv_register( &usb_host_drv );
 
    lv_obj_t * mouse_cursor = lv_img_create(lv_scr_act());
    lv_img_set_src(mouse_cursor, LV_SYMBOL_PLAY);
    //lv_img_set_src(mouse_cursor, "Cursor_11x19");  // does't work yet
    lv_indev_set_cursor(mouse_indev, mouse_cursor);

/*
    // Set a cursor for the mouse
    LV_IMG_DECLARE(mouse_cursor_icon)
    lv_obj_t * cursor_obj = lv_img_create(lv_scr_act());      // Create an image object for the cursor 
    lv_img_set_src(cursor_obj, &mouse_cursor_icon);           // Set the image source
    lv_indev_set_cursor(mouse_indev, cursor_obj);             // Connect the image  object to the driver
*/
     /*KeyPad
     Later you should create group(s) with `lv_group_t * group = lv_group_create()`,
     *add objects to the group with `lv_group_add_obj(group, obj)`
     *and assign this input device to group to navigate in it:
     *`lv_indev_set_group(indev_keypad, group);`*/

    Serial.printf("\nUSB Soft Host Test for '%s'.\n", PROFILE_NAME );
    USH.init( USB_Pins_Config, my_USB_DetectCB, my_USB_PrintCB);
    #ifdef SHOW_DETAILS
      printf("-------------------------------------------\n");
    #endif

    cb_queue = xQueueCreate( 10, sizeof(struct cbMessage) );

#if 0
    /* Create simple label */
    lv_obj_t *label = lv_label_create( lv_scr_act() );
    lv_label_set_text( label, LVGL_Arduino.c_str() );
    lv_obj_align( label, LV_ALIGN_CENTER, 0, 0 );
#else
    /* Try an example from the lv_examples Arduino library
       make sure to include it as written above.
    lv_example_btn_1();
   */

    //// modified !!!

    // uncomment only **one** of these demos
    lv_demo_widgets(); Serial.println( "lv_demo_widgets();" );                // OK
    // lv_demo_benchmark(); Serial.println( "lv_demo_benchmark();" );            // OK
    // lv_demo_keypad_encoder(); Serial.println( "lv_demo_keypad_encoder();" );  // works, but I haven't an encoder
    // lv_demo_music();              // NOK
    // lv_demo_printer();
    // lv_demo_stress();             // seems to be OK
#endif
    Serial.println( "Setup done" );
}
// ------------------------------------------------------------------------------------------ //
void loop()
{
    lv_timer_handler(); /* let the GUI do its work */
    delay( 5 );
}
//http://lvgl.100ask.org/8.2/index.html
//https://www.cnblogs.com/frozencandles/archive/2022/06/14/16375392.html
// ------------------------------------------------------------------------------------------ //

