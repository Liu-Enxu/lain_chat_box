// RP2040 Earle's package 4.4.2 works best

#include <TFT_eSPI.h>
#include <SPI.h>
#include "User_Setup_Select.h"
#include <SD.h>
#include "FS.h"
#include "hardware/pll.h"
#include "hardware/clocks.h"

// TFT ------------------------------------------------------------------
// #include "redlain.h"
// #include "bluelain.h"

// #define animation_width 128
// #define animation_height 128

TFT_eSPI tft = TFT_eSPI(TFT_WIDTH,TFT_HEIGHT);
TFT_eSprite img = TFT_eSprite(&tft);
uint16_t* imgPtr;
// uint16_t  dmaBuffer1[16*16];
// uint16_t* dmaBufferPtr = dmaBuffer1;


// SD ------------------------------------------------------------------
#define _CS0 17
#define _SCK0 18
#define _MOSI0 19
#define _MISO0 16

#define _CS1 13
#define _SCK1 14
#define _MOSI1 15
#define _MISO1 12

SDClass SD0;
SDClass SD1;
SDClass* SDs[2] = {&SD0,&SD1};

File file;
File file0;
File file1;
File* files[2] = {&file0,&file1};

bool SD0FrameRead = false;
bool SD1FrameRead = false;
bool SDFrameRead[2] = {SD0FrameRead,SD1FrameRead};

uint8_t MSB,LSB;
uint16_t color_val;


// SPISettings spisettings(23000000, MSBFIRST, SPI_MODE0);

// var ------------------------------------------------------------------
bool TFT_init = false;
// int x = 0;
int idx = -1;
bool rev = false;

unsigned short frame[16384] = {0};

void setup() {
  // set_sys_clock_khz(260000,true);
  Serial.begin(115200);

  // delay(5000);
  // Serial.print("Initializing SD card on bus 0...");
  // SPI.setCS(_CS0);
  // SPI.setRX(_MISO0);
  // SPI.setTX(_MOSI0);
  // SPI.setSCK(_SCK0);
  // pinMode(_CS0,OUTPUT);
  // SPI.begin(true);
  // if (!SD0.begin(_CS0,SPI)) {
  //   Serial.println("initialization of bus0 SD failed!");
  //   while (1);
  // }
  // Serial.println("initialization of bus0 SD done.");

  delay(5000);
  Serial.print("Initializing SD card on bus 1...");
  SPI1.setCS(_CS1);
  SPI1.setRX(_MISO1);
  SPI1.setTX(_MOSI1);
  SPI1.setSCK(_SCK1);
  pinMode(_CS1,OUTPUT);
  SPI1.begin(true);
  // SPI1.beginTransaction(spisettings);
  // SPI1.endTransaction();
  if (!SD1.begin(_CS1,20000000,SPI1)) { // 23000000
    Serial.println("initialization of bus1 SD failed!");
    while (1);
  }
  Serial.println("initialization of bus1 SD done."); 

  tft.init(); //TFT_BLACK
  tft.initDMA();
  tft.fillScreen(TFT_BLACK);
  tft.setRotation(0);
  tft.writecommand(TFT_MADCTL);
  tft.writedata(TFT_MAD_MX | TFT_MAD_MY | TFT_MAD_COLOR_ORDER);
  tft.setSwapBytes(true); 
  
  // img.setSwapBytes(true); // comment out if DMA; use if normal sprite
  img.setColorDepth(16);
  // img.createSprite(TFT_WIDTH, TFT_HEIGHT);
  imgPtr = (uint16_t*)img.createSprite(TFT_WIDTH, TFT_HEIGHT);
  // img.setTextDatum(MC_DATUM);
  tft.startWrite();


  delay(100);
}

void loop() {

  // test1 -------------------------------------
  // tft.fillRect(0,0,TFT_WIDTH,TFT_HEIGHT,TFT_BLACK);
  // delay(1000);
  // tft.fillRect(0,0,TFT_WIDTH,TFT_HEIGHT,TFT_WHITE);
  // delay(1000);

  // test2 -------------------------------------
  // img.fillCircle(x, x, 20, TFT_BLACK);
  // img.fillRect(64,64,100,100,TFT_BLACK);

  // x=x+1;if(x>128){x=0;}
  // img.fillCircle(x, x, 20, TFT_RED);
  
  // img.setTextColor(TFT_WHITE);
  // img.drawString(String(x),64,64,6);

  // img.pushSprite(0,0);
  

  // read from flashed header -----------------
  // for(int i=0;i<frames;i++){
  //   delay(30);
  //   idx = i;
  //   if(rev){idx = frames-1-i;}
  //   tft.pushImage(0,0,TFT_WIDTH,TFT_HEIGHT,frame[idx]);
    // img.pushImage(0,0,TFT_WIDTH,TFT_HEIGHT,frame[idx]);
    // img.pushSprite(0,0);
  // }
  // rev = !rev;

  // read from SD card ------------------------
  // readSDAndPush("redlain");
  // readSDAndPush("bluelain",&SD1);
  readASCII("bad_appleASCII", &SD1);
  // while (1);
  // read from dual SD card ------------------------
  // readSD("b",0);
}

void setup1() {
  // Serial.begin(115200);
  // delay(5000);
  // Serial.print("Initializing SD card on bus 1...");
  // SPI1.setRX(_MISO1);
  // SPI1.setTX(_MOSI1);
  // SPI1.setSCK(_SCK1);
  // pinMode(_CS1,OUTPUT);
  // if (!SD1.begin(_CS1,23000000,SPI1)) { // 23000000
  //   Serial.println("initialization of bus1 SD failed!");
  //   while (1);
  // }
  // Serial.println("initialization of bus1 SD done."); 
}

void loop1() {
  // readSD("b",1);
}

File openFile(String str,SDClass* sd){
  // file name
  const String filename = str + ".bin"; // or txr
  // const char* fileName = filename.c_str();
  
  // check if exists
  if (!sd->exists(filename)) {
    Serial.println(filename+" doesn't exist.");
    File file;
    return file;
  } else {
    Serial.println(filename+" exists.");
  }

  // open file
  File myFile = sd->open(filename, FILE_READ);
  if (!myFile) {
    Serial.println("SD Card: error on opening file "+filename);
    File file;
    return file;
  }

  return myFile;
}

void closeFile(File* file){
  file->close();
}

void readASCII(String str,SDClass* sd){
  // open file
  file = openFile(str,sd);
  // read file
  byte buffer[4096]; int bytesRead = 0; // 2048 seem to be limit?

  int pixelIdx = 0;
  unsigned long end;
  unsigned long start = millis();

  while (file.available()) {
    bytesRead = file.readBytes((char*)buffer, sizeof(buffer)); // read characters one by one from Micro SD Card
    for(int i=0;i<bytesRead;i+=2){
      MSB = buffer[i];
      LSB = buffer[i+1];
      color_val = (MSB << 8) | LSB;
      // Serial.println("color val: "+String(color_val));

      frame[pixelIdx] = color_val;
      pixelIdx = (pixelIdx+1)%(TFT_WIDTH*TFT_HEIGHT);
      if(!pixelIdx){
        end = millis(); Serial.println("spf:"+String(end-start));
        // tft.pushImage(0,0,TFT_WIDTH,TFT_HEIGHT,frame);

        // img.pushImage(0,0,TFT_WIDTH,TFT_HEIGHT,(uint16_t *)frame,16U);
        // img.pushSprite(0,0);

        img.fillSprite(TFT_GREEN);
        img.pushImage(0, 0, TFT_WIDTH,TFT_HEIGHT,(uint16_t *)frame,16U);
        tft.pushImageDMA(0, 0, TFT_WIDTH, TFT_HEIGHT, imgPtr);

        start = end;
      } 
    } 
  }
  closeFile(&file);
}

void readSDAndPush(String str,SDClass* sd){
  // open file
  file = openFile(str,sd);
  // read file
  char buffer[4096]; int bytesRead = 0; // 2048 seem to be limit?
  char word[4]; 
  int pixelIdx = 0;
  unsigned long end;
  unsigned long start = millis();

  while (file.available()) {
    bytesRead = file.readBytes(buffer, sizeof(buffer)); // read characters one by one from Micro SD Card
    for(int i=0;i<bytesRead;i+=4){
      strncpy(word, &buffer[i], 4);
      uint16_t num = strtol(word, NULL, 16);memset(word, 0, sizeof(word));
      frame[pixelIdx] = num;
      pixelIdx = (pixelIdx+1)%(TFT_WIDTH*TFT_HEIGHT);
      if(!pixelIdx){
        end = millis(); Serial.println("spf:"+String(end-start));
        tft.pushImage(0,0,TFT_WIDTH,TFT_HEIGHT,frame);
        
        // img.pushImage(0,0,TFT_WIDTH,TFT_HEIGHT,(uint16_t *)frame,16U);
        // img.pushSprite(0,0);
        
        // tft.pushImageDMA(0, 0, TFT_WIDTH, TFT_HEIGHT, frame, dmaBufferPtr);
        // start = millis();
        start = end;
      } 
    } 
  }
  closeFile(&file);
}

void readSD(String str, int ID){
  // open file
  *(files[ID]) = openFile(str,SDs[ID]);
  // read file
  char buffer[4096]; int bytesRead = 0; // 2048 seem to be limit?
  char word[4]; 
  int pixelIdx = 0;
  unsigned long end;
  unsigned long start = millis();

  while (files[ID]->available()) {
    bytesRead = files[ID]->readBytes(buffer, sizeof(buffer)); // read characters one by one from Micro SD Card
    for(int i=0;i<bytesRead;i+=4){
      strncpy(word, &buffer[i], 4);
      uint16_t num = strtol(word, NULL, 16);memset(word, 0, sizeof(word));
      frame[pixelIdx+ID*(TFT_WIDTH*TFT_HEIGHT)/2] = num;
      pixelIdx = (pixelIdx+1)%(TFT_WIDTH*TFT_HEIGHT/2);
      if(!pixelIdx){
        SDFrameRead[ID] = true;
        while(!(SDFrameRead[0] && SDFrameRead[1]));
        end = millis(); Serial.println(String(ID)+"-spf:"+String(end-start)); start = end;
        if(!ID){
          tft.pushImage(0,0,TFT_WIDTH,TFT_HEIGHT,frame);
          // tft.pushImageDMA(0, 0, TFT_WIDTH, TFT_HEIGHT, imgPtr);  
        }
        // start = millis();
        SDFrameRead[ID] = false; 
      } 
    } 
  }
  closeFile(&file);
}