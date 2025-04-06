#pragma once
#include <TFT_eSPI.h>
#include "setupSD.h"
#include "setupButton.h"
#include "setupBuiltInGIF.h"
#include "setupFreeRTOS.h"

// TFT ------------------------------------------------------------------

TFT_eSPI tft = TFT_eSPI(TFT_WIDTH, TFT_HEIGHT);
TFT_eSprite img = TFT_eSprite(&tft);
uint16_t *imgPtr;

// var ------------------------------------------------------------------
bool TFT_init = false;

byte buffer[4096];
int bytesRead = 0;
int pixelIdx = 0;
// unsigned short frame[16384] = {0};//frame array
static unsigned short frames[2][16384] = { 0 };
uint8_t frame_buf_id = 0;
uint8_t current_buf_id;
unsigned long end_TFT = 0;    // fps
unsigned long start_TFT = 0;  // fps

uint8_t idx;
uint8_t delayTFT;
// int x = 0;
// bool rev = false;
String gif_name = "output.bin";  // readSD need this!!!!!
flashedGifPara *gifPara = &idle_gifPara;


void initTFT() {

  // TFT
  tft.init();  //TFT_BLACK
  tft.initDMA();
  tft.fillScreen(TFT_BLACK);
  tft.setRotation(0);
  tft.writecommand(TFT_MADCTL);
  tft.writedata(TFT_MAD_MX | TFT_MAD_MY | TFT_MAD_COLOR_ORDER);
  tft.setSwapBytes(true);
  img.setTextColor(TFT_BLUE);

  // img.setSwapBytes(true); // comment out if DMA; use if normal sprite
  img.setColorDepth(16);
  // img.createSprite(TFT_WIDTH, TFT_HEIGHT);
  imgPtr = (uint16_t *)img.createSprite(TFT_WIDTH, TFT_HEIGHT);
  // img.setTextDatum(MC_DATUM);
  tft.startWrite();
}

// read bin header
void headerTask(void *params) {
  while (1) {
    start_TFT = millis();
    for (int i = 0; i < 2 * (gifPara->framesNum); i++) {
      
      idx = (i < gifPara->framesNum) ? i : (2 * (gifPara->framesNum) - i - 1);
      if (xSemaphoreTake(xSemaphoreHeader_Restart, 0) == pdTRUE) { break; }  // Exit the loop and restart to read other header
      img.pushImage(0, 0, TFT_WIDTH, TFT_HEIGHT, (gifPara->frames[idx]));
      delayTFT = ((gifPara->frameDelay)>(millis()-start_TFT))?((gifPara->frameDelay)-(millis()-start_TFT)):0;
      vTaskDelay(pdMS_TO_TICKS(delayTFT));
      end_TFT = millis();
      
      img.drawString("ms/f: " + String(end_TFT - start_TFT), 0, 0, 2);
      img.drawString("Header: " + String(uxTaskGetStackHighWaterMark(header_Task_Handle)), 0, 12, 2);
      img.drawString("Record: " + String(uxTaskGetStackHighWaterMark(recording_Task_Handle)), 0, 24, 2);

      tft.pushImageDMA(0, 0, TFT_WIDTH, TFT_HEIGHT, imgPtr);
      tft.dmaWait();

      start_TFT = end_TFT;
    }
  }
};

// read bin RTOS
void SDTask(void *params) {
  // read animationFile
  bytesRead = 0;
  pixelIdx = 0;

  while (1) {
    // open animationFile
    // xSemaphoreTake(xSemaphoreSD_Done, 0);
    openFile(gif_name, &animationFile);
    pixelIdx = 0;
    // read the file
    start_TFT = millis();
    while (animationFile.available()) {

      if (xSemaphoreTake(xSemaphoreSD_Restart, 0) == pdTRUE) {
        // Serial.println("Restarting SDTask...");
        closeFile(&animationFile);  // Close file safely
        break;                      // Exit the loop and restart reading with new gif_name
      }

      bytesRead = animationFile.readBytes((char *)buffer, sizeof(buffer));  // read characters one by one from Micro SD Card
      for (int i = 0; i < bytesRead; i += 2) {
        MSB = buffer[i];
        LSB = buffer[i + 1];
        color_val = (MSB << 8) | LSB;
        // Serial.println("color val: "+String(color_val));

        frames[frame_buf_id][pixelIdx] = color_val;
        pixelIdx = (pixelIdx + 1) % (TFT_WIDTH * TFT_HEIGHT);
        if (!pixelIdx) {
          // xSemaphoreTake(xbufID_Mutex, portMAX_DELAY);
          frame_buf_id = !frame_buf_id;  // Flip buffer ID
          // xSemaphoreGive(xbufID_Mutex);

          xSemaphoreGive(xSemaphoreSD_Done);  // Signal TFT task
          // xSemaphoreTake(xSemaphoreTFT_Done, portMAX_DELAY);      // Wait for TFT to finish
        }
      }
    }
    // close the file
    closeFile(&animationFile);

    // pdMS_TO_TICKS(10);
    // xSemaphoreGive(xSemaphoreFile_Done);
    // vTaskDelete(TFT_Task_Handle);
    // vTaskDelete(NULL);
  }
};

void TFTTask(void *params) {
  while (1) {
    // RTOS -------------------------------------------
    xSemaphoreTake(xSemaphoreSD_Done, portMAX_DELAY);  // Wait for TFT to finish

    end_TFT = millis();  //Serial.println("spf:"+String(end_TFT-start_TFT));
    // push image -------------------------------------------
    // tft.pushImage(0,0,TFT_WIDTH,TFT_HEIGHT,frame0);

    // img.pushImage(0,0,TFT_WIDTH,TFT_HEIGHT,(uint16_t *)frame0,16U);
    // img.pushSprite(0,0);

    // xSemaphoreTake(xbufID_Mutex, portMAX_DELAY);
    // current_buf_id = !frame_buf_id; // Read the safe buffer index
    // xSemaphoreGive(xbufID_Mutex);

    img.pushImage(0, 0, TFT_WIDTH, TFT_HEIGHT, (uint16_t *)(frames[!frame_buf_id]), 16U);

    img.drawString("ms/f: " + String(end_TFT - start_TFT), 0, 0, 2);
    img.drawString("SD: " + String(uxTaskGetStackHighWaterMark(SD_Task_Handle)), 0, 12, 2);
    img.drawString("TFT: " + String(uxTaskGetStackHighWaterMark(TFT_Task_Handle)), 0, 24, 2);

    tft.pushImageDMA(0, 0, TFT_WIDTH, TFT_HEIGHT, imgPtr);
    tft.dmaWait();

    // iterate -------------------------------------------
    start_TFT = end_TFT;

    // RTOS -------------------------------------------
    // xSemaphoreGive(xSemaphoreTFT_Done);      // Signal TFT task
  }
};

void readbinRTOS() {
  //RTOS
  // if (SD_Task_Handle != nullptr) {vTaskDelete(SD_Task_Handle);SD_Task_Handle = nullptr;}
  // if (TFT_Task_Handle != nullptr) {vTaskDelete(TFT_Task_Handle);TFT_Task_Handle = nullptr;}

  xTaskCreate(TFTTask, "TFT_Task", 512, nullptr, configMAX_PRIORITIES - 2, &TFT_Task_Handle);
  vTaskSuspend(TFT_Task_Handle);
  xTaskCreate(SDTask, "SD_Task", 2048, nullptr, configMAX_PRIORITIES - 1, &SD_Task_Handle);
  vTaskResume(TFT_Task_Handle);
  // vTaskCoreAffinitySet(TFT_Task_Handle, 1 << 1);
}
