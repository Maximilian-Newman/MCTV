/*Copyright (c) 2026 Maximilian Newman Loussouarn

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.



Fonts from: https://github.com/Avamander/arduino-tvout
*/




#include <TVout.h>
#include "fontALL.h"
#include "MemoryFree.h"



const String MAC = "001403060E46"; // 14:3:60e46
const String DEVICE_NAME = "MCTV";
const String DEVICE_TYPE = ",TV,";

unsigned long lastPing = 0;
unsigned long lastTopBarUpdate = 0;

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





String tvMode = "";




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

void fillRect(byte x1, byte y1, byte x2, byte y2, byte c) {
  for (byte y=y1; y<=y2; y++) {
    for (byte x=x1; x<=x2; x++){
      TV.set_pixel(x, y, c);
    }
  }
}

void print(String s){
  for (unsigned int i=0; i<s.length(); i++){
    TV.print(s[i]);
  }
}

void updateTopBar() {
  fillRect(0, 0, 199, 6, 0);
  TV.select_font(font4x6);
  TV.set_cursor(0, 0);
  TV.print("MCTV - ");
  if (isConnected()) {
    TV.print("connected: ");
    for (byte i=0; i<6; i++) {
      TV.print(connectedAddress[i], HEX);
      TV.print(" ");
    }
    TV.print(" - mode: ");
    print(tvMode);
  }
  TV.draw_line(0, 7, 199, 7, 1);

  if (millis() - lastPing > 9000) {
    TV.set_cursor(196, 0);
    TV.print('!');
  }
  lastTopBarUpdate = millis();
}

void debugParams(int a, int b, int c, int d, int e) {
  Serial.print(a); Serial.print(" ");
  Serial.print(b); Serial.print(" ");
  Serial.print(c); Serial.print(" ");
  Serial.print(d); Serial.print(" ");
  Serial.println(e);
}



void setup(){
  TV.begin(PAL, 200, 200);
  mainScreen();
  Serial3.begin(115200);
  Serial.begin(115200);
  Serial3.print("DISCONNECT\n");
  //Serial3.setTimeout(50);
}

void loop(){
  TV.delay_frame(1);

  if (Serial3.available()){
    lastPing = millis();
    String command = Serial3.readStringUntil('\n');
    Serial.println("COMMAND: " + command);
    if (command != "PING") {Serial3.print("1\n");}
    



    if (command == "CONNECT"){
      Serial.println();
      convertMAC(Serial3.readStringUntil('\n'), connectedAddress);
      TV.clear_screen();
    }

    else if (command == "DISCONNECT"){
      for (byte i=0; i<6; i++) {
        connectedAddress[i] = 0;
      }
      Serial.println("disconnected");
      mainScreen();
    }



    else if (command == "PING") {
      Serial3.print("PING\n");
    }

    else if (command == "NAME") {
      Serial3.print("NAME:" + DEVICE_NAME + "\n");
    }

    else if (command == "TYPE") {
      Serial3.print("TYPE:" + DEVICE_TYPE + "\n");
    }

    else if (!isConnected()) {
      Serial3.print("DISCONNECT\n");
    }








    else if (command == "TV.LINE") {
      int x1 = Serial3.readStringUntil(',').toInt();
      int y1 = Serial3.readStringUntil(',').toInt() + 8;
      int x2 = Serial3.readStringUntil(',').toInt();
      int y2 = Serial3.readStringUntil(',').toInt() + 8;
      byte c = Serial3.readStringUntil('\n').toInt() - '0';
      TV.draw_line(x1, y1, x2, y2, c);

      debugParams(x1, y1, x2, y2, c);
    }

    else if (command == "TV.FILLRECT") {
      byte x1 = Serial3.readStringUntil(',').toInt();
      byte y1 = Serial3.readStringUntil(',').toInt() + 8;
      byte x2 = Serial3.readStringUntil(',').toInt();
      byte y2 = Serial3.readStringUntil(',').toInt() + 8;
      byte c = Serial3.readStringUntil('\n').toInt() - '0';

      if (x1 > x2){
        byte temp = x1;
        x1 = x2;
        x2 = temp;
      }
      if (y1 > y2){
        byte temp = y1;
        y1 = y2;
        y2 = temp;
      }

      fillRect(x1, y1, x2, y2, c);

      debugParams(x1, y1, x2, y2, c);
    }

    else if (command == "TV.RECT") {
      int x1 = Serial3.readStringUntil(',').toInt();
      int y1 = Serial3.readStringUntil(',').toInt() + 8;
      int x2 = Serial3.readStringUntil(',').toInt();
      int y2 = Serial3.readStringUntil(',').toInt() + 8;
      byte c = Serial3.readStringUntil('\n').toInt() - '0';

      if (x1 > x2){
        byte temp = x1;
        x1 = x2;
        x2 = temp;
      }
      if (y1 > y2){
        byte temp = y1;
        y1 = y2;
        y2 = temp;
      }
      TV.draw_rect(x1, y1, x2-x1, y2-y1, c);

      debugParams(x1, y1, x2, y2, c);
    }

    else if (command == "TV.PIXEL") {
      int x = Serial3.readStringUntil(',').toInt();
      int y = Serial3.readStringUntil(',').toInt() + 8;
      byte c = Serial3.readStringUntil('\n').toInt() - '0';
      TV.set_pixel(x, y, c);

      debugParams(x, y, 0, 0, c);
    }

    else if (command == "TV.CLEAR") {
      byte c = Serial3.readStringUntil('\n').toInt() - '0';
      fillRect(0, 8, 199, 199, c);
      
      debugParams(0, 0, 0, 0, c);
    }

    else if (command == "TV.PRINT") {
      int x = Serial3.readStringUntil(',').toInt();
      int y = Serial3.readStringUntil(',').toInt() + 8;
      byte fontNum = Serial3.readStringUntil(',').toInt();
      String s = Serial3.readStringUntil('\n');

      if (fontNum == 1) {TV.select_font(font4x6);}
      else if (fontNum == 2) {TV.select_font(font6x8);}
      else if (fontNum == 3) {TV.select_font(font8x8);}
      else if (fontNum == 4) {TV.select_font(font8x8ext);}

      TV.set_cursor(x, y);
      print(s);
    }

    else if (command == "TV.MODE") {
      tvMode = Serial3.readStringUntil('\n');
    }

    else if (command == "TV.BIM") {
      byte x0 = Serial3.readStringUntil(',').toInt();
      byte y0 = Serial3.readStringUntil(',').toInt() + 8;
      byte width = Serial3.readStringUntil(',').toInt();
      byte height = Serial3.readStringUntil(',').toInt();

      Serial.println(x0);
      Serial.println(y0);
      Serial.println(width);
      Serial.println(height);

      for (byte y=y0; y<y0+height; y++) {
        for (byte x = x0; x<x0+width; x++) {
          byte timeoutCounter = 0;
          while (!Serial3.available()) {
            timeoutCounter += 1;
            delay(1);
            if (timeoutCounter > 250) {goto SKIP_BIM;}
          }
          byte c = Serial3.read() - '0';
          Serial.print(c);

          if (c<3) {TV.set_pixel(x, y, c);}
        }
        Serial.println(" <-");
      }
    }









    else {
      Serial.println("unknown command: " + command);
    }
  }

  SKIP_BIM:


  if (millis() - lastPing > 60000 and isConnected()) {
    for (byte i=0; i<6; i++) {
      connectedAddress[i] = 0;
    }
    Serial.println("disconnected");
    mainScreen();
  }

  if (isConnected() and millis() - lastTopBarUpdate > 2000) {
    updateTopBar();
  }
}
