// modified kbdparser.h from https://github.com/tobozo/ESP32-USB-Soft-Host

#define KEY_QUEUE_LENGTH 10

// ------------------------------------------------------------------------------------------ //
class KbdRptParser : public KeyboardReportParser
{
  void PrintKey(uint8_t mod, uint8_t key);
  char lastkey[2] = {0,0};
  xQueueHandle key_queue = NULL;
  uint8_t Key_Queue_Length = 10;

  protected:
    void OnControlKeysChanged (uint8_t before, uint8_t after);
    void OnKeyDown	          (uint8_t mod, uint8_t key);
    void OnKeyUp	            (uint8_t mod, uint8_t key);
    void OnKeyPressed         (uint8_t key);

  public:
    void     InitKeyQueue(uint8_t length);
    uint8_t  MessagesWaiting();
    uint32_t ReceiveKey();
    void     SendKey(uint32_t key);
};
// ------------------------------------------------------------------------------------------ //
void KbdRptParser::InitKeyQueue( uint8_t length )
{ 
  if (length > 5) 
    Key_Queue_Length = length;
  key_queue = xQueueCreate( Key_Queue_Length, sizeof( uint32_t ) );
}
// ------------------------------------------------------------------------------------------ //
uint8_t KbdRptParser::MessagesWaiting()
{
  return uxQueueMessagesWaiting( key_queue );
}
// ------------------------------------------------------------------------------------------ //
uint32_t KbdRptParser::ReceiveKey()
{
  uint32_t key = 0;
     
  xQueueReceive(key_queue, &key, ( portTickType ) 10 );
  return key;
}// ------------------------------------------------------------------------------------------ //
void KbdRptParser::SendKey(uint32_t key)
{
  xQueueSend(key_queue, &key, (TickType_t) 0);
}
// ------------------------------------------------------------------------------------------ //
void KbdRptParser::PrintKey(uint8_t m, uint8_t key)
{
#ifdef DEBUG_KBD  
  MODIFIERKEYS mod;
  *((uint8_t*)&mod) = m;
  Serial.print((mod.bmLeftCtrl   == 1) ? "C" : " ");
  Serial.print((mod.bmLeftShift  == 1) ? "S" : " ");
  Serial.print((mod.bmLeftAlt    == 1) ? "A" : " ");
  Serial.print((mod.bmLeftGUI    == 1) ? "G" : " ");

  Serial.print(" >");
  Serial.printf("0x%02x", key );
  Serial.print("< ");

  Serial.print((mod.bmRightCtrl   == 1) ? "C" : " ");
  Serial.print((mod.bmRightShift  == 1) ? "S" : " ");
  Serial.print((mod.bmRightAlt    == 1) ? "A" : " ");
  Serial.println((mod.bmRightGUI  == 1) ? "G" : " ");
#endif
};
// ------------------------------------------------------------------------------------------ //
void KbdRptParser::OnKeyDown(uint8_t mod, uint8_t key)
{
#ifdef DEBUG_KBD    
  Serial.print("DN ");
  PrintKey(mod, key);
#endif  
  uint8_t c = OemToAscii(mod, key);
  if (c)
    OnKeyPressed(c);
}
// ------------------------------------------------------------------------------------------ //
void KbdRptParser::OnControlKeysChanged(uint8_t before, uint8_t after) {

#ifndef DEBUG_KBD  
  return;
#endif
  MODIFIERKEYS beforeMod;
  *((uint8_t*)&beforeMod) = before;

  MODIFIERKEYS afterMod;
  *((uint8_t*)&afterMod) = after;

  if (beforeMod.bmLeftCtrl != afterMod.bmLeftCtrl) {
    Serial.println("LeftCtrl changed");
  }
  if (beforeMod.bmLeftShift != afterMod.bmLeftShift) {
    Serial.println("LeftShift changed");
  }
  if (beforeMod.bmLeftAlt != afterMod.bmLeftAlt) {
    Serial.println("LeftAlt changed");
  }
  if (beforeMod.bmLeftGUI != afterMod.bmLeftGUI) {
    Serial.println("LeftGUI changed");
  }

  if (beforeMod.bmRightCtrl != afterMod.bmRightCtrl) {
    Serial.println("RightCtrl changed");
  }
  if (beforeMod.bmRightShift != afterMod.bmRightShift) {
    Serial.println("RightShift changed");
  }
  if (beforeMod.bmRightAlt != afterMod.bmRightAlt) {
    Serial.println("RightAlt changed");
  }
  if (beforeMod.bmRightGUI != afterMod.bmRightGUI) {
    Serial.println("RightGUI changed");
  }
}
// ------------------------------------------------------------------------------------------ //
void KbdRptParser::OnKeyUp(uint8_t mod, uint8_t key)
{
#ifdef DEBUG_KBD  
  Serial.print("UP ");
  PrintKey(mod, key);
#endif  
}
// ------------------------------------------------------------------------------------------ //
int ux = 0;
void KbdRptParser::OnKeyPressed(uint8_t key)
{ 
  int num = 0;
  uint32_t k;

  k = key;
  
  if (k > 0x7f)
   switch (key) {
      case 0xa4 : k = 0xc3a4; break; // "ä"   // works, if you have a font, which have these characters ???
      case 0x84 : k = 0xc384; break; // "Ä"
      case 0xb6 : k = 0xc3b6; break; // "ö"
      case 0x96 : k = 0xc396; break; // "Ö"
      case 0xbc : k = 0xc32f; break; // "ü"
      case 0x9c : k = 0xc39c; break; // "Ü"
      case 0xb0 : k = 0xc2b0; break; // "°"
      case 0xb2 : k = 0xc2b2; break; // "²"
      case 0xb3 : k = 0xc2b3; break; // "³"
      case 0xb5 : k = 0xc2b5; break; // "µ"
      case 0x9f : k = 0xc39f; break; // "ß"
      case 0xb4 : k = 0xc2b4; break; // "´"
      case 0xac : k = 0xE282AC; break; // "€"
    }
  
  if (k > 0)
    xQueueSend(key_queue, &k, (TickType_t) 0);
  //num = uxQueueMessagesWaiting( key_queue );
  //if (ux < num) ux = num;
  //Serial.printf("%i", num); 

  #ifdef DEBUG_KBD    
    Serial.printf("ASCII(0x%02x): ", key);
  #endif  

  if (key > 0x7e) {
    if      (key==0xa4) Serial.print("ä");  // german keyboard
    else if (key==0x84) Serial.print("Ä");
    else if (key==0xb6) Serial.print("ö");
    else if (key==0x96) Serial.print("Ö");
    else if (key==0xbc) Serial.print("ü");
    else if (key==0x9c) Serial.print("Ü");
    else if (key==0xb0) Serial.print("°");
    else if (key==0xac) Serial.print("€");
    else if (key==0xb2) Serial.print("²");
    else if (key==0xb3) Serial.print("³");
    else if (key==0xb5) Serial.print("µ");
    else if (key==0x9f) Serial.print("ß");
    else if (key==0xb4) Serial.print("´");
    else if (key==0x7f) Serial.print("'DEL'");
    else Serial.print((char)key);
  }
  else if (key < 0x20) {
    if      (key==0x0B) Serial.print("'SHIFT+TAB'");  // german keyboard
    else if (key==0x1b) Serial.print("'ESC'");
    else if (key==0x08) Serial.print("'BACKSPACE'");
    else if (key==0x14) Serial.print("'LEFT'");
    else if (key==0x13) Serial.print("'RIGHT'");
    else if (key==0x11) Serial.print("'UP'");
    else if (key==0x12) Serial.print("'DOWN'");
    else if (key==0x02) Serial.print("'HOME'");
    else if (key==0x03) Serial.print("'END'");
    else Serial.print((char)key);
  }
  else if (key==0x60) 
    Serial.print("`");
  else {
    Serial.print((char)key);
  }  
  lastkey[0] = (char)key;
#ifdef DEBUG_KBD    
  Serial.println();
#endif 
};
// ------------------------------------------------------------------------------------------ //