// RP2040 Earle's package 4.4.2 at least

// include heap_4.c for heap monitoring

// #include "setupFreeRTOS.h"
#include "setupTFT.h"
#include "setupSPI.h"
#include "setupSD.h"
#include "setupI2S.h"
#include "setupButton.h"
#include "setupWireless.h"

void managerTask(void *pvParameters){
  (void) pvParameters;  
  while(1){
    if(detectStatus()){
      if(!isRecording){
        xSemaphoreGive(xSemaphoreRecording_Done);
        while(!(eTaskGetState(recording_Task_Handle)==eDeleted)); // until confirmed
        save_wav();
        Serial.println("Saved!\n");

        // file transmission
        gifPara = &think_gifPara;xSemaphoreGive(xSemaphoreHeader_Restart);

        // sendFile(myWavName,&soundFile);
        xTaskCreate(sendSoundTask, "sendSoundTask", 512, nullptr, configMAX_PRIORITIES - 1, &sendSound_Task_Handle);
        while(!(eTaskGetState(sendSound_Task_Handle)==eDeleted));
        // receiveFile();
        xTaskCreate(recvSoundTask, "recvSoundTask", 512, nullptr, configMAX_PRIORITIES - 1, &recvSound_Task_Handle);
        while(!(eTaskGetState(recvSound_Task_Handle)==eDeleted));

        gifPara = &speak_gifPara;xSemaphoreGive(xSemaphoreHeader_Restart);
        play_wav(reWavName);
        
        // idle
        gifPara = &idle_gifPara;xSemaphoreGive(xSemaphoreHeader_Restart);

      } else {
        // recording
        gifPara = &listen_gifPara;xSemaphoreGive(xSemaphoreHeader_Restart);
        vTaskSuspend(header_Task_Handle);
        xTaskCreate(recordingTask, "recordingTask", 512, nullptr, configMAX_PRIORITIES - 1, &recording_Task_Handle);
        vTaskResume(header_Task_Handle);
      }
    };
  }
}

void setup() {
  Serial.begin(115200);
  delay(5000);

  
  //RTOS
  checkFreeRTOS();
  // printTaskInfo();
  createSemaphore();
  delay(1000);
  // SD
  Serial.print("Initializing SD card on bus 1...");
  initSPI1();
  initSD1();
  Serial.println("initialization of bus1 SD done."); 
  // deleteFile(myWavName);
  deleteFile(reWavName);
  // I2S
  initI2SIn();
  initI2SOut();
  // Button
  initButton();
  // Wireless
  initWireless();
  connectPC();
  // TFT
  initTFT();
  delay(100);

  xTaskCreate(managerTask, "managerTask", 1024, nullptr, configMAX_PRIORITIES - 3, &manager_Task_Handle);
  xTaskCreate(headerTask, "headerTask", 512, gifPara, configMAX_PRIORITIES - 2, &header_Task_Handle);
  
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

