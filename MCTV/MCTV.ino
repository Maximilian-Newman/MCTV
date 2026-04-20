
#include <TVout.h>
#include "fontALL.h"



const String MAC = "001403060E46"; // 14:3:60e46
const String DEVICE_NAME = "MCTV";
const String DEVICE_TYPE = ",TV,";

unsigned long lastPing = 0;

TVout TV;
uint8_t connectedAddress[6]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

bool isConnected() {
  for (byte i=0; i<6; i++) {
    if (connectedAddress[i] != 0){
      return true;
    }
  }
  return false;
}

void print(String s){
  for (unsigned int i=0; i<s.length(); i++){
    TV.print(s[i]);
  }
}

byte nibble(char c){
  if (c >= '0' && c <= '9')
    return c - '0';

  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;

  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;

  return 0;  // Not a valid hexadecimal character
}
void convertMAC(String hexString, byte byteArray[]){
  byte currentByte = 0;
  byte byteIndex = 0;

  for (byte charIndex = 0; charIndex < 12; charIndex++){
    bool oddCharIndex = charIndex & 1;

    if (!oddCharIndex)
    {
      // Odd characters go into the high nibble
      currentByte = nibble(hexString[charIndex]) << 4;
    }
    else
    {
      // Odd characters go into low nibble
      currentByte |= nibble(hexString[charIndex]);
      byteArray[byteIndex] = currentByte;
      currentByte = 0;
      byteIndex += 1;
    }
  }
}

void mainScreen(){
  TV.clear_screen();
  TV.select_font(font8x8);
  TV.set_cursor(90, 0);
  TV.print("MCTV");
  TV.select_font(font6x8);
  TV.set_cursor(30, 50);
  TV.print("Use bluetooth to connect");
  TV.set_cursor(70, 100);
  TV.print("MAC Address:");
  TV.draw_rect(55, 110, 100, 25, 1);
  TV.set_cursor(70, 120);
  print(MAC);

  TV.set_cursor(0, 0);
}



void setup(){
  TV.begin(PAL, 200, 200);
  mainScreen();
  Serial3.begin(115200);
}

void loop(){
  if (Serial3.available()){
    lastPing = millis();
    String command = Serial3.readStringUntil('\n');
    Serial.println("COMMAND: " + command);



    if (command == "CONNECT"){
      Serial.println();
      convertMAC(Serial3.readStringUntil('\n'), connectedAddress);
      
      for (byte i=0; i<6; i++){
        Serial.print(connectedAddress[i], HEX);
        Serial.print(" ");
      }
      Serial.println();

      TV.clear_screen();
      TV.select_font(font8x8);
      TV.set_cursor(90, 0);
      TV.print("MCTV");
      TV.select_font(font6x8);
      TV.set_cursor(30, 50);
      TV.print("Succesfully connected!");
      TV.set_cursor(70, 100);
      for (byte i=0; i<6; i++){
        TV.print(connectedAddress[i], HEX);
        TV.print(" ");
      }

    }

    else if (command == "DISCONNECT"){
      for (byte i=0; i<6; i++) {
        connectedAddress[i] = 0;
      }
      Serial.println("disconnected");
      mainScreen();
    }

    else if (command == "NAME") {
      Serial3.print("NAME:" + DEVICE_NAME + "\n");
    }

    else if (command == "TYPE") {
      Serial3.print("TYPE:" + DEVICE_TYPE + "\n");
    }

    else if (command == "PING") {
      Serial3.print("PING\n");
    }








    
    else if (command == "TV.CLEAR") {
      TV.clear_screen();
    }

    else if (command.startsWith("TV.LINE")) {
      int x1 = Serial3.readStringUntil(',').toInt();
      int y1 = Serial3.readStringUntil(',').toInt();
      int x2 = Serial3.readStringUntil(',').toInt();
      int y2 = Serial3.readStringUntil(',').toInt();
      byte c = Serial3.readStringUntil('\n').toInt();
      TV.draw_line(x1, y1, x2, y2, c);
    }








    else {
      Serial.println("unknown command: " + command);
    }
  }


  if (millis() - lastPing > 60000 and isConnected()) {
    for (byte i=0; i<6; i++) {
      connectedAddress[i] = 0;
    }
    Serial.println("disconnected");
    mainScreen();
  }
}
