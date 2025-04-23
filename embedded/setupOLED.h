#pragma once
#include <U8g2lib.h>
// #include "setupBuiltInGIF.h"
#include "lain_gaze.h"
#include "setupI2C.h"

typedef U8G2_SSD1306_128X64_NONAME_F_HW_I2C OLED;

OLED OLED_print(U8G2_R2, U8X8_PIN_NONE);
OLED OLED_left(U8G2_R0, U8X8_PIN_NONE);
OLED OLED_middle(U8G2_R0, U8X8_PIN_NONE);
OLED OLED_right(U8G2_R0, U8X8_PIN_NONE);

OLED* allScreenPtr[4] = {&OLED_print,&OLED_left,&OLED_middle,&OLED_right};
uint8_t char_size[4] = {0,0,0,0};

uint8_t getCharWidth(OLED* myScreen){
  // get the char width
  uint8_t pos = 0;
  myScreen->setCursor(0, 0);
  myScreen->firstPage();
  do {
    myScreen->print("一");
    while(1){
      pos = myScreen->getCursorX();
      // Serial.println(pos);
      if(pos){
        myScreen->setCursor(0, 0);
        return pos;
      }
    }
  } while ( myScreen->nextPage() );
  
}

void addTextPageOLED(OLED* myScreen, uint8_t char_width, String myStr, String speaker){
  // start new page
  myScreen->firstPage();
  do {
    myScreen->setCursor(0, char_width);
    if(speaker!=""){
      for(int i=0;i<speaker.length();i++){
        myScreen->print(speaker.substring(i,i+1));
      }myScreen->print(":");
    }
    for(int i=0;i<myStr.length();i++){
      if(myScreen->getCursorX()+char_width>myScreen->getDisplayWidth()){
        if(myScreen->getCursorY()+char_width>myScreen->getDisplayHeight()){
          myScreen->clearBuffer();myScreen->setCursor(0, 0);
        }
        myScreen->setCursor(0, myScreen->getCursorY()+char_width);
      } 
      myScreen->print(myStr.substring(i,i+1));
    }

  } while ( myScreen->nextPage() );
}

void addTextPageOLED(uint8_t screen_id, uint8_t char_width, String myStr, String speaker){
  select_MUX(screen_id);
  allScreenPtr[screen_id]->firstPage();
  do {
    allScreenPtr[screen_id]->setCursor(0, char_width);
    if(speaker!=""){
      for(int i=0;i<speaker.length();i++){
        allScreenPtr[screen_id]->print(speaker.substring(i,i+1));
      }allScreenPtr[screen_id]->print(":");
    }
    for(int i=0;i<myStr.length();i++){
      if(allScreenPtr[screen_id]->getCursorX()+char_width>allScreenPtr[screen_id]->getDisplayWidth()){
        if(allScreenPtr[screen_id]->getCursorY()+char_width>allScreenPtr[screen_id]->getDisplayHeight()){
          allScreenPtr[screen_id]->clearBuffer();allScreenPtr[screen_id]->setCursor(0, 0);
        }
        allScreenPtr[screen_id]->setCursor(0, allScreenPtr[screen_id]->getCursorY()+char_width);
      } 
      allScreenPtr[screen_id]->print(myStr.substring(i,i+1));
    }

  } while ( allScreenPtr[screen_id]->nextPage() );
}

void addTextBufferOLED(OLED* myScreen, uint8_t char_width, String myStr, String speaker,bool newline){
  myScreen->setCursor(myScreen->getCursorX(), myScreen->getCursorY());
  if (newline){
    if(myScreen->getCursorY()+char_width>myScreen->getDisplayHeight()){
      myScreen->clearBuffer();myScreen->setCursor(0, char_width);
    } else {
      myScreen->setCursor(0, myScreen->getCursorY()+char_width);
    } 
  }
  if(speaker!=""){
    for(int i=0;i<speaker.length();i++){
      myScreen->print(speaker.substring(i,i+1));
      myScreen->sendBuffer();
    }myScreen->print(":");myScreen->sendBuffer();
  }
  for(int i=0;i<myStr.length();i++){
    if(myScreen->getCursorX()+char_width>myScreen->getDisplayWidth()){
      if(myScreen->getCursorY()+char_width>myScreen->getDisplayHeight()){
        myScreen->clearBuffer();myScreen->setCursor(0, 0);
      }
      myScreen->setCursor(0, myScreen->getCursorY()+char_width);
    } 
    myScreen->print(myStr.substring(i,i+1));
    myScreen->sendBuffer();
  }
}

void addTextBufferOLED(uint8_t screen_id, uint8_t char_width, String myStr, String speaker,bool newline){
  select_MUX(screen_id);
  allScreenPtr[screen_id]->setCursor(allScreenPtr[screen_id]->getCursorX(), allScreenPtr[screen_id]->getCursorY());
  if (newline){
    if(allScreenPtr[screen_id]->getCursorY()+char_width>allScreenPtr[screen_id]->getDisplayHeight()){
      allScreenPtr[screen_id]->clearBuffer();allScreenPtr[screen_id]->setCursor(0, char_width);
    } else {
      allScreenPtr[screen_id]->setCursor(0, allScreenPtr[screen_id]->getCursorY()+char_width);
    } 
  }
  if(speaker!=""){
    for(int i=0;i<speaker.length();i++){
      allScreenPtr[screen_id]->print(speaker.substring(i,i+1));
      allScreenPtr[screen_id]->sendBuffer();
    }allScreenPtr[screen_id]->print(":");allScreenPtr[screen_id]->sendBuffer();
  }
  for(int i=0;i<myStr.length();i++){
    if(allScreenPtr[screen_id]->getCursorX()+char_width>allScreenPtr[screen_id]->getDisplayWidth()){
      if(allScreenPtr[screen_id]->getCursorY()+char_width>allScreenPtr[screen_id]->getDisplayHeight()){
        allScreenPtr[screen_id]->clearBuffer();allScreenPtr[screen_id]->setCursor(0, 0);
      }
      allScreenPtr[screen_id]->setCursor(0, allScreenPtr[screen_id]->getCursorY()+char_width);
    } 
    allScreenPtr[screen_id]->print(myStr.substring(i,i+1));
    allScreenPtr[screen_id]->sendBuffer();
  }
}

#define debugOLED(msg, spkr, newline) addTextBufferOLED(uint8_t(0), char_size[0], String(msg), String(spkr), newline)

void bootAnimation(OLED* myScreen){
  unsigned long lastUpdateTime = 0;
  const unsigned long fadeDelay = 100;
  myScreen->setFont(u8g2_font_boutique_bitmap_9x9_bold_t_gb2312a);
  for(int j=0;j<5;j++){
    for(int i=0;i<lain_gaze_framesNum;i++){
      myScreen->firstPage();
      do {
        // myScreen->clearBuffer();
        myScreen->drawXBMP(0, 0, lain_gaze_width, lain_gaze_height, lain_gaze_Frames[i]);
        myScreen->drawStr(lain_gaze_width, 40, "Lain OS");
        // myScreen->sendBuffer();
      } while ( myScreen->nextPage() );  
      delay(100);
    }
  }

  delay(1000);
  
  uint8_t height = myScreen->getDisplayHeight();
  uint8_t width_bit = height;
  uint8_t width_byte = int(width_bit/8);
  uint8_t x_rand;uint8_t y_rand;

  uint8_t fade_buffer[width_byte*height];
  memcpy(fade_buffer,lain_gaze_Frames[lain_gaze_framesNum-1],width_byte*height);
  lastUpdateTime = millis();
  while (millis() - lastUpdateTime < 4e3) {
    myScreen->firstPage();
    do{
      for(int i=0;i<150;i++){
        x_rand = random(width_bit);y_rand = random(height);
        fade_buffer[int(x_rand)/8+y_rand*width_byte] &= uint8_t( ~(1 << (7 - x_rand % 8)) ); // or just 0 for 8 pixel in a row
      }
      myScreen->drawXBM(0, 0, width_bit, height, fade_buffer);
    } while ( myScreen->nextPage() );
  }
  myScreen->clearBuffer();myScreen->sendBuffer();
}

void initOLED(){
  for(int i=0;i<sizeof(allScreenPtr) / sizeof(allScreenPtr[0]);i++){
    select_MUX(i);
    allScreenPtr[i]->begin();  
    allScreenPtr[i]->enableUTF8Print();
    
    select_MUX(i);
    if(!i){bootAnimation(allScreenPtr[i]);}
    allScreenPtr[i]->setFont(u8g2_font_boutique_bitmap_7x7_t_gb2312a);  // use chinese2 for all the glyphs of "你好世界"
    allScreenPtr[i]->setFontDirection(0);

    char_size[i] = getCharWidth(allScreenPtr[i]);
    allScreenPtr[i]->clearBuffer();
    // allScreenPtr[i]->setCursor(0, char_size[i]);
    // addTextPageOLED(allScreenPtr[i],char_size[i], "");
    
  }
  debugOLED("启动中...","Sys",true);  
}



