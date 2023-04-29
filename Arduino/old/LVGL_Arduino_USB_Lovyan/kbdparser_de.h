// modified kbdparser.h from https://github.com/tobozo/ESP32-USB-Soft-Host

class KbdRptParser : public KeyboardReportParser
{
  void PrintKey(uint8_t mod, uint8_t key);
  char lastkey[2] = {0,0};
  protected:
    void OnControlKeysChanged(uint8_t before, uint8_t after);
    void OnKeyDown	(uint8_t mod, uint8_t key);
    void OnKeyUp	(uint8_t mod, uint8_t key);
    void OnKeyPressed(uint8_t key);
};
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
void KbdRptParser::OnKeyPressed(uint8_t key)
{
#ifdef DEBUG_KBD    
  Serial.printf("ASCII(0x%02x): ", key);
#endif  
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
  else if (key==0x60) Serial.print("`");
  else
    Serial.print((char)key);
  lastkey[0] = (char)key;
#ifdef DEBUG_KBD    
  Serial.println();
#endif 
};
// ------------------------------------------------------------------------------------------ //