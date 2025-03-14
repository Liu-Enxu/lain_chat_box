// RP2040 Earle's package 4.4.2 works best
// #include "setupFreeRTOS.h"
#include "setupTFT.h"
#include "setupSPI.h"
#include "setupSD.h"
#include "setupI2S.h"
#include "setupButton.h"

void changeGIFTask(void *pvParameters){
  (void) pvParameters;
  
  while(1){
    // Serial.print("Free Heap: " + String(xPortGetFreeHeapSize()) + "bytes; ");
    // Serial.println("History Low Heap: " + String(xPortGetMinimumEverFreeHeapSize()) + "bytes");
    Serial.print("recording task: " + String(uxTaskGetStackHighWaterMark(recording_Task_Handle)) + " words");
    Serial.print(";SD task: " + String(uxTaskGetStackHighWaterMark(SD_Task_Handle)) + " words");
    Serial.println(";TFT task: " + String(uxTaskGetStackHighWaterMark(TFT_Task_Handle)) + " words");
    if(detectStatus()){
      if(!isRecording){
        gif_name = "bluelain";
        // xSemaphoreGive(xSemaphoreRecording_Done);
      } else {
        gif_name = "redlain";
        xSemaphoreGive(xSemaphoreSD_Restart);
        // xSemaphoreGive(xSemaphoreRecording_Start);
      }

    };
  }
}

void setup() {
  Serial.begin(115200);
  delay(5000);
  
  //RTOS
  checkFreeRTOS();
  createSemaphore();
  delay(1000);
  // SD
  Serial.print("Initializing SD card on bus 1...");
  initSPI1();
  initSD1();
  Serial.println("initialization of bus1 SD done."); 
  // I2S
  initI2SIn();
  // Button
  initButton();
  // TFT
  initTFT();

  delay(100);

  
  if (changeGIF_Task_Handle != nullptr) {vTaskDelete(changeGIF_Task_Handle);changeGIF_Task_Handle = nullptr;}
  xTaskCreate(changeGIFTask, "changeGIFTask", 512, nullptr, configMAX_PRIORITIES - 4, &changeGIF_Task_Handle);
  // vTaskCoreAffinitySet(changeGIF_Task_Handle, 1 << 1);

  // Don't bother same priority, it won't work // need TOP PRIORITY or DONT WORK???
  if (recording_Task_Handle != nullptr) {vTaskDelete(recording_Task_Handle);recording_Task_Handle = nullptr;}
  xTaskCreate(recordingTask, "recordingTask", 3072, nullptr, configMAX_PRIORITIES - 1, &recording_Task_Handle);
  // vTaskCoreAffinitySet(recording_Task_Handle, 1 << 0);

  readbinRTOS();
  
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
  
  // ???????????????????????????????????????????
  // file = openFile("bad_appleASCII",&SD1);
  // read from flashed header -----------------
  // for(int i=0;i<frames;i++){
  //   delay(30);
  //   idx = i;
  //   if(rev){idx = frames-1-i;}
  //   tft.pushImage(0,0,TFT_WIDTH,TFT_HEIGHT,Frames[idx]);
  //   img.pushImage(0,0,TFT_WIDTH,TFT_HEIGHT,Frames[idx]);
  //   img.pushSprite(0,0);
  // }
  // rev = !rev;

  

  // read from SD card ------------------------
  // readSDAndPush("redlain");
  // readSDAndPush("bluelain",&SD1);

  

  // readbin("bluelain", &SD1);

  // while (1);
  // read from dual SD card ------------------------
  // readSD("b",0);

  // test audio ----------------------------------
  // if (Serial.available() > 0) {
  //   String command = Serial.readStringUntil('\n');
  //   command.trim();
  //   if (command == "rec") {
  //     isRecording = true;
  //   } else {
  //     baseFileName = command;
  //     fileNumber = 1; // reset file number each time a new base file name is set
  //     Serial.printf("Send rec for starting recording label \n");
  //   }
  // }
  // if (isRecording && baseFileName != "") {
  //   String fileName = "/" + baseFileName + "_" + String(fileNumber) + ".wav";
  //   Serial.println("filename: "+fileName);
  //   if (SD1.exists(fileName.c_str())) {  // Check if the file exists
  //     SD1.remove(fileName.c_str());      // Delete the file
  //     Serial.println("Old file deleted.");
  //   }
  //   fileNumber++;
  //   record_wav(fileName);
  //   delay(1000); // delay to avoid recording multiple files at once
  //   isRecording = false;
  // }

}

