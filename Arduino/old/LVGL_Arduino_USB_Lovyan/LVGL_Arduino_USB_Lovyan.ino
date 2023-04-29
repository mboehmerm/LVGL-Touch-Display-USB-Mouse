/////////////////////////////////////////////////////////////////
/*
  ESP32 | LVGL8 | Ep 0. GFX Setup (ft. LovyanGFX)
  Video Tutorial: https://youtu.be/IPCvQ4o_WP8
  Created by Eric N. (ThatProject)
  Modified by mboehmerm
*/
/////////////////////////////////////////////////////////////////

//#define DEBUG_ALL
//#define DEBUG_KBD
//#define FORCE_TEMPLATED_NOPS

//#define lang_DE  // German keyboard

#include <arduino.h>

#include <lvgl.h>
//#define LGFX_USE_V1
#include <LovyanGFX.hpp>

#include <demos/lv_demos.h>
#include <examples/lv_examples.h>  //#include <../examples/lv_examples.h>

// Only for PlatformIO. PlatformIO doesn't find *.c files in subdirectories ? Arduino IDE does. 
//#define PLATFORMIO
#ifdef PLATFORMIO
  #include <examples/widgets/btn/lv_example_btn_1.c>
  #include <examples/widgets/msgbox/lv_example_msgbox_1.c>
  #include <examples/widgets/menu/lv_example_menu_1.c>
  #include <examples/widgets/tabview/lv_example_tabview_1.c>
  #include <examples/get_started/lv_example_get_started_1.c>
  // ...
#endif

//#define LGFX_ESP32_ILI9341_CS14
#ifdef LGFX_ESP32_ILI9341_CS14
  #include <../../LGFX_ESP32_ILI9341_CS14.hpp>  // Path Arduino/libraries/ or LVGL_Demo_PIO/.pio/libdeps/esp32dev/
#else  
//  Create a class that does your own settings, derived from LGFX_Device. 
class LGFX : public lgfx::LGFX_Device
{ 
  lgfx::Panel_ILI9341      _panel_instance;
  lgfx::Bus_SPI            _bus_instance;
  lgfx::Touch_XPT2046      _touch_instance;

public:

  LGFX(void) 
  { 
    {  //  Configure bus control settings. 
      auto cfg = _bus_instance.config();  //  Get the structure for the bus configuration. 

      //  SPI bus configuration 
      cfg.spi_host    = VSPI_HOST;        //  Select the SPI to use ESP32-S2,C3 : SPI2_HOST or SPI3_HOST / ESP32 : VSPI_HOST or HSPI_HOST 
      //  * Due to the ESP-IDF version upgrade, VSPI_HOST and HSPI_HOST descriptions are deprecated, so if an error occurs, use SPI2_HOST and SPI3_HOST instead. 
      cfg.spi_mode    = 0;                //  Set SPI communication mode (0 ~ 3)
      cfg.freq_write  = 40000000;         //  80000000 // SPI clock frequency when sending (max 80MHz, rounded to 80MHz divided by an integer)
      cfg.freq_read   = 16000000;         //  SPI clock frequency when receiving 
      cfg.spi_3wire   = true;             //  Set true if receiving on the MOSI 
      cfg.use_lock    = true;             //  set true to use transaction 
      cfg.dma_channel = SPI_DMA_CH_AUTO;  //  Set the DMA channel to use (0=not use DMA / 1=1ch / 2=ch / SPI_DMA_CH_AUTO=auto setting) 
      //  * With the ESP-IDF version upgrade, SPI_DMA_CH_AUTO (automatic setting) is recommended for the DMA channel.   Specifying 1ch and 2ch is deprecated. 
      cfg.pin_sclk    = 18;               //  SPI SCLK
      cfg.pin_mosi    = 23;               //  SPI MOSI
      cfg.pin_miso    = 19;               //  SPI MISO      (-1 = disable) 
      cfg.pin_dc      =  2;               //  SPI D/C = A0  (-1 = disable) 
      //  When using the same SPI bus as the SD card, be sure to set MISO without omitting it. 
      
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    { // Set display panel controls.
      auto cfg = _panel_instance.config();

      cfg.pin_cs           =    14;  // CS    (-1 = disable)
      cfg.pin_rst          =    -1;  // RST   (-1 = disable)
      cfg.pin_busy         =    -1;  // BUSY  (-1 = disable)

      // * The following setting values ​​are general initial values ​​for each panel, so please try commenting out any unknown items. 

      cfg.panel_width      =   240;  // actual displayable width
      cfg.panel_height     =   320;  // actual displayable  height
      cfg.offset_x         =     0;  // Panel offset in X 
      cfg.offset_y         =     0;  // Panel offset in Y
      cfg.offset_rotation  =     2;  // Rotation value offset 0~7 (4~7 are upside down 
      cfg.dummy_read_pixel =     8;  // Number of dummy read bits before pixel read
      cfg.dummy_read_bits  =     1;  // Number of dummy read bits before non-pixel data read
      cfg.readable         =  true;  // Data can be read set to true
      cfg.invert           = false;  // if panel light and dark are inverted set to true
      cfg.rgb_order        = false;  // if panel red and blue are reversed set to true
      cfg.dlen_16bit       = false;  // Set to true for panels that transmit data length in 16-bit units with 16-bit parallel or SPI
      cfg.bus_shared       =  true;  // If the bus is shared with the SD card, set to true (bus control with drawJpgFile etc.)

      // Please set below only with drivers that can change the number of pixels such as ST7735 and ILI9163.
      // Please set the following only when the display shifts with a driver with a variable number of pixels such as ST7735 or ILI9163.
      //cfg.memory_width     =   240;  // Maximum width  supported by the driver IC
      //cfg.memory_height    =   320;  // Maximum height supported by the driver IC

      _panel_instance.config(cfg);
    }

    { // Configure settings for touch screen control.  (delete if not necessary)

      auto cfg = _touch_instance.config();

      // numbers for x_min etc. doesn't work
      cfg.x_min           =  340;  // Minimum X value (raw value) obtained from the touchscreen
      cfg.x_max           = 3900;  // Maximum X value (raw value) obtained from the touchscreen
      cfg.y_min           =  235;  // Minimum Y value obtained from touchscreen (raw value)
      cfg.y_max           = 3900;  // Maximum Y value (raw value) obtained from touchscreen
      cfg.pin_int         =   -1;  // Pin number to which INT is connected (-1 = not connected)
      cfg.bus_shared      = true;  // Set true when using a common bus with the screen
      cfg.offset_rotation =    0;  // Adjustment when the orientation does not match Set with a value from 0 to 7
      //cfg.offset_rotation =  6;  // 6 fits but double lines. why?

      // For SPI connection
      cfg.spi_host = VSPI_HOST;    // Select SPI to use (HSPI_HOST or VSPI_HOST)
      cfg.freq     = 2500000;      // SPI Clock frequency 1000000 -> 2500000
      cfg.pin_sclk = 18;           // SCLK
      cfg.pin_mosi = 23;           // MOSI
      cfg.pin_miso = 19;           // MISO
      cfg.pin_cs   =  4;           // CS

      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance);
    }
    setPanel(&_panel_instance);
  }
};
#endif

LGFX tft;

#define SCREEN_ROTATION 0                                   // set the screen rotation

/*Change to your screen resolution*/
#if (SCREEN_ROTATION == 1) || (SCREEN_ROTATION == 3)
  static const uint16_t screenWidth  = 320;                 // rotation 1 or 3
  static const uint16_t screenHeight = 240;
#else  
  static const uint16_t screenWidth  = 240;                 // rotation 0 or 2
  static const uint16_t screenHeight = 320;
#endif

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[ screenWidth * screenHeight / 4 ];    // screen buffer size
//static lv_color_t buf[ screenWidth * 10 ];                // smaller if compile error

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
   //tft.pushColors( ( uint16_t * )&color_p->full, w * h, true );
   tft.writePixels((lgfx::rgb565_t *)&color_p->full, w * h);
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
}// ------------------------------------------------------------------------------------------ //
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
      data->point.x = touchX;
      data->point.y = touchY;

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
  Serial.begin(115200);

  tft.begin();        
  tft.setRotation(SCREEN_ROTATION);
  tft.setBrightness(255);

  uint16_t calData[8] = {3890, 340, 3900, 3895, 235, 340, 235, 3900};
  tft.setTouchCalibrate(calData);

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

  /*Initialize the (dummy) input device driver
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);*/

    Serial.printf("\nUSB Soft Host Test for '%s'.\n", PROFILE_NAME );
    USH.init( USB_Pins_Config, my_USB_DetectCB, my_USB_PrintCB);
    #ifdef SHOW_DETAILS
      printf("-------------------------------------------\n");
    #endif

    cb_queue = xQueueCreate( 10, sizeof(struct cbMessage) );

  // *** uncomment only **ONE** of these lines ( examples or demos ) ***
    
  // lv_example_get_started_1();
  // lv_example_btn_1();            
  // lv_example_msgbox_1();
  // lv_example_menu_1();
  // lv_example_tabview_1();
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
// ------------------------------------------------------------------------------------------ //
/*
static void btn_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * btn = lv_event_get_target(e);
    if(code == LV_EVENT_CLICKED) {
        static uint8_t cnt = 0;
        cnt++;

        // Get the first child of the button which is the label and change its text
        lv_obj_t * label = lv_obj_get_child(btn, 0);
        lv_label_set_text_fmt(label, "Button: %d", cnt);
    }
}

// Create a button with a label and react on click event.
void lv_example_get_started_1(void)
{
    lv_obj_t * btn = lv_btn_create(lv_scr_act());    
    lv_obj_set_size(btn, 120, 50);
    lv_obj_align(btn, LV_ALIGN_CENTER, 0,0);
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, NULL);

    lv_obj_t * label = lv_label_create(btn);
    lv_label_set_text(label, "Button");
    lv_obj_center(label);
}
*/
// ------------------------------------------------------------------------------------------ //