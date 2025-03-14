#pragma once
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>


TaskHandle_t recording_Task_Handle;
SemaphoreHandle_t xSemaphoreRecording_Done;

TaskHandle_t changeGIF_Task_Handle;
SemaphoreHandle_t xSemaphoreSD_Done;TaskHandle_t SD_Task_Handle;
// SemaphoreHandle_t xSemaphoreFile_Done;
SemaphoreHandle_t xSemaphoreSD_Restart;
SemaphoreHandle_t xSemaphoreTFT_Done;TaskHandle_t TFT_Task_Handle;
SemaphoreHandle_t xbufID_Mutex;


// static bool tasksCreated = false;


void checkFreeRTOS(){
#if defined(configVERSION)
  Serial.print("FreeRTOS Version: ");
  Serial.println(configVERSION);
#else
  Serial.println("FreeRTOS Version not defined");
#endif
  Serial.print("Base task number: ");
  Serial.println(uxTaskGetNumberOfTasks());
}

void createSemaphore(){
  xSemaphoreRecording_Done = xSemaphoreCreateBinary();
  xSemaphoreSD_Done = xSemaphoreCreateBinary(); xSemaphoreSD_Restart = xSemaphoreCreateBinary();
  xSemaphoreTFT_Done = xSemaphoreCreateBinary();
  xbufID_Mutex = xSemaphoreCreateMutex();
  if (xSemaphoreRecording_Done == NULL || xSemaphoreSD_Done == NULL || xSemaphoreTFT_Done == NULL || xSemaphoreSD_Restart == NULL || xbufID_Mutex == NULL) {
    Serial.println("Failed to create semaphores!");
    while (1);  // Stop execution if semaphore creation fails
  }
}