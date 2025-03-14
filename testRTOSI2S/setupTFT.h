#pragma once
#include <TFT_eSPI.h>
#include "setupSD.h"
#include "setupButton.h"
#include "setupFreeRTOS.h"

// TFT ------------------------------------------------------------------
// #include "redlain.h"
// #include "bluelain.h"

TFT_eSPI tft = TFT_eSPI(TFT_WIDTH,TFT_HEIGHT);
TFT_eSprite img = TFT_eSprite(&tft);
uint16_t* imgPtr;

// var ------------------------------------------------------------------
bool TFT_init = false;
int x = 0;
int idx = -1;
bool rev = false;

// unsigned short frame[16384] = {0};//frame array
static unsigned short frames[2][16384] = {0};
uint8_t frame_buf_id = 0;
uint8_t current_buf_id;
unsigned long end = 0;    // fps
unsigned long start = 0;  // fps
String gif_name = "bluelain"; // default

void initTFT(){
  
  // TFT
  tft.init(); //TFT_BLACK
  tft.initDMA();
  tft.fillScreen(TFT_BLACK);
  tft.setRotation(0);
  tft.writecommand(TFT_MADCTL);
  tft.writedata(TFT_MAD_MX | TFT_MAD_MY | TFT_MAD_COLOR_ORDER);
  tft.setSwapBytes(true);img.setTextColor(TFT_BLUE);
  
  // img.setSwapBytes(true); // comment out if DMA; use if normal sprite
  img.setColorDepth(16);
  // img.createSprite(TFT_WIDTH, TFT_HEIGHT);
  imgPtr = (uint16_t*)img.createSprite(TFT_WIDTH, TFT_HEIGHT);
  // img.setTextDatum(MC_DATUM);
  tft.startWrite();
}

// read bin RTOS
void SDTask(void *params) {
  // String* str = (String*)params;

  // read animationFile
  byte buffer[4096]; 
  int bytesRead = 0; // 2048 seem to be limit?
  int pixelIdx = 0;
  
  while(1){
    // open animationFile
    xSemaphoreTake(xSemaphoreSD_Done, 0);
    openFile(gif_name,&animationFile);pixelIdx = 0;
    // read the file
    start = millis();
    while (animationFile.available()) {

      if (xSemaphoreTake(xSemaphoreSD_Restart, 0) == pdTRUE) {
          // Serial.println("Restarting SDTask...");
          closeFile(&animationFile);  // Close file safely
          break;  // Exit the loop and restart reading with new gif_name
      }

      bytesRead = animationFile.readBytes((char*)buffer, sizeof(buffer)); // read characters one by one from Micro SD Card
      for(int i=0;i<bytesRead;i+=2){
        MSB = buffer[i];
        LSB = buffer[i+1];
        color_val = (MSB << 8) | LSB;
        // Serial.println("color val: "+String(color_val));

        frames[frame_buf_id][pixelIdx] = color_val;
        pixelIdx = (pixelIdx+1)%(TFT_WIDTH*TFT_HEIGHT);
        if(!pixelIdx){
          // xSemaphoreTake(xbufID_Mutex, portMAX_DELAY);
          frame_buf_id = !frame_buf_id; // Flip buffer ID
          // xSemaphoreGive(xbufID_Mutex); 

          xSemaphoreGive(xSemaphoreSD_Done);                       // Signal TFT task
          // xSemaphoreTake(xSemaphoreTFT_Done, portMAX_DELAY);      // Wait for TFT to finish
        }
      }
    }
    // close the file
    closeFile(&animationFile);
    // xSemaphoreGive(xSemaphoreFile_Done);
    // vTaskDelete(TFT_Task_Handle);
    // vTaskDelete(NULL);
  }
};

void TFTTask(void *params) {
  while(1){
    // RTOS -------------------------------------------
    xSemaphoreTake(xSemaphoreSD_Done, portMAX_DELAY);    // Wait for TFT to finish

    end = millis(); //Serial.println("spf:"+String(end-start));
    // push image -------------------------------------------
    // tft.pushImage(0,0,TFT_WIDTH,TFT_HEIGHT,frame0);

    // img.pushImage(0,0,TFT_WIDTH,TFT_HEIGHT,(uint16_t *)frame0,16U);
    // img.pushSprite(0,0);

    // xSemaphoreTake(xbufID_Mutex, portMAX_DELAY);
    // current_buf_id = !frame_buf_id; // Read the safe buffer index
    // xSemaphoreGive(xbufID_Mutex);

    img.pushImage(0, 0, TFT_WIDTH,TFT_HEIGHT,(uint16_t *)(frames[!frame_buf_id]),16U);
    img.drawString(String(end-start),64,64,6);
    tft.pushImageDMA(0, 0, TFT_WIDTH, TFT_HEIGHT, imgPtr);
    tft.dmaWait();

    // iterate -------------------------------------------
    start = end;

    // RTOS -------------------------------------------
    // xSemaphoreGive(xSemaphoreTFT_Done);      // Signal TFT task
  }
};

void readbinRTOS(){
  //RTOS
  if (SD_Task_Handle != nullptr) {vTaskDelete(SD_Task_Handle);SD_Task_Handle = nullptr;}
  if (TFT_Task_Handle != nullptr) {vTaskDelete(TFT_Task_Handle);TFT_Task_Handle = nullptr;}

  xTaskCreate(SDTask, "SD_Task", 3072, nullptr, configMAX_PRIORITIES - 1, &SD_Task_Handle);
  xTaskCreate(TFTTask, "TFT_Task", 2048, nullptr, configMAX_PRIORITIES - 3, &TFT_Task_Handle);
}

