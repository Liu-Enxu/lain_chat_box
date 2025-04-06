#include <TFT_eSPI.h>
#include <SPI.h>
#include <SD.h>

// #include <stdint.h>
// #include <stdio.h>

// TFT ------------------------------------------------------------------

// #include "redlain.h"
// #include "bluelain.h"

// #ifdef USE_DMA
//   uint16_t  dmaBuffer1[16*16]; // Toggle buffer for 16*16 MCU block, 512bytes
//   uint16_t  dmaBuffer2[16*16]; // Toggle buffer for 16*16 MCU block, 512bytes
//   uint16_t* dmaBufferPtr = dmaBuffer1;
//   bool dmaBufferSel = 0;
// #endif

#define ST7735_DRIVER
#define SPI_FREQUENCY  27000000
#define TFT_WIDTH  130
#define TFT_HEIGHT 130

#define TFT_CS   PIN_D8  // Chip select control pin D8
#define TFT_DC   PIN_D3  // Data Command control pin
#define TFT_RST  PIN_D4

#define TFT_SCLK PIN_D5// SCK
#define TFT_WR   PIN_D7 // SDA

#define animation_width 128
#define animation_height 128

TFT_eSPI tft = TFT_eSPI(TFT_WIDTH,TFT_HEIGHT);
TFT_eSprite img = TFT_eSprite(&tft);

// SD ------------------------------------------------------------------
#define MISO     PIN_D6
#define PIN_SPI_CS 10
fs::File myFile;
// var ------------------------------------------------------------------
int x = 0;
int idx = -1;
bool rev = false;

int frameNum = 8;
// unsigned short frame[][16384] = {0};
unsigned short frame[16384] = {0};

void openFile(char str[]){
  // file name
  String filename = String(str) + ".txt";
  const char* fileName = filename.c_str();
  
  // check if exists
  if (SD.exists(fileName)) {
    // Serial.println(filename+" exists.");
  } else {
    // Serial.println(filename+" doesn't exist.");
    return;
  }

  // open file
  myFile = SD.open(fileName, FILE_READ);
  if (!myFile) {
    Serial.println("SD Card: error on opening file "+filename);
    return;
  }
}

void closeFile(){
  myFile.close();
}

// void readSDOnce(char str[]){
//   // open file
//   openFile(str);
//   // read file
//   char word[5] = {}; int wordIdx = 0; int frameIdx = 0; int pixelIdx = 0;
//   while (myFile.available()) {
//     char ch = myFile.read(); // read characters one by one from Micro SD Card
//     if(ch=='\n'){
//       Serial.println("Frame ended!");
//       frameIdx++; pixelIdx = 0;
//       if(frameIdx==1){break;}
//     } else if(ch==' '){
//       int num = atoi(word);
//       frame[frameIdx][pixelIdx] = num;
//       pixelIdx++;
//       Serial.println(num); // print the character to Serial Monitor
//       char word[5] = {}; wordIdx = 0;
//     } else {
//       word[wordIdx++] = ch;
//     }
//   }
//   Serial.println();
//   Serial.println("Read finished.");
//   closeFile();
// }

void readAndPushSD(char str[]){
  // open file
  openFile(str);
  // read file
  char word[5] = {}; int wordIdx = 0; int pixelIdx = 0;
  unsigned long start = millis();

  while (myFile.available()) {
    char ch = myFile.read(); // read characters one by one from Micro SD Card
    if(ch=='\n'){
      unsigned long end = millis(); Serial.println("Time to read frame:"+String(end-start));
      // Serial.println("Frame ended!");
      tft.pushImage(0,0,animation_width,animation_height,frame);
      pixelIdx = 0;
      start = millis();
    } else if(ch==' '){
      // int num = atoi(word);
      uint16_t num = strtol(word, NULL, 16);
      frame[pixelIdx] = num;
      pixelIdx++;
      // Serial.println(num); // print the character to Serial Monitor
      memset(word, 0, sizeof(word)); wordIdx = 0;
    } else {
      word[wordIdx++] = ch;
    }
  }
  closeFile();
  // Serial.println();
  // Serial.println("Read finished.");
}

void setup() {
  Serial.begin(115200);
  delay(5000);
  Serial.print("Initializing SD card...");
  if (!SD.begin(PIN_SPI_CS)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");

  

  tft.init(); //TFT_BLACK
  tft.fillScreen(TFT_BLACK);
  tft.setRotation(0);
  tft.writecommand(TFT_MADCTL);
  tft.writedata(TFT_MAD_MX | TFT_MAD_MY | TFT_MAD_COLOR_ORDER);
  tft.setSwapBytes(true);
  img.createSprite(TFT_WIDTH, TFT_HEIGHT);

  delay(100);
}

void loop() {
  // test -------------------------------------
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
  //   tft.pushImage(0,0,animation_width,animation_height,frame[idx]);
  // }
  // rev = !rev;

  // read from SD card ------------------------
  // for(int i=0;i<1;i++){
  //   delay(40);
  //   idx = i;
  //   if(rev){idx = 1-1-i;}
  //   tft.pushImage(0,0,animation_width,animation_height,frame[idx]);
  // }
  // rev = !rev;
  readAndPushSD("redlain");
  
}
