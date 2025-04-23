#pragma once
#include "setupOLED.h"
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include "hardware/watchdog.h"

#define configCHECK_FOR_STACK_OVERFLOW 2

// monitoring
// TaskHandle_t stackMonitor_Task_Handle;
TaskHandle_t manager_Task_Handle;

// recording
TaskHandle_t recording_Task_Handle;
SemaphoreHandle_t xSemaphoreRecording_Saved;
SemaphoreHandle_t xSemaphoreRecording_Done;

// displaying
TaskHandle_t header_Task_Handle;
SemaphoreHandle_t xSemaphoreHeader_Restart;

// TaskHandle_t SD_Task_Handle;
// SemaphoreHandle_t xSemaphoreSD_Done;
// SemaphoreHandle_t xSemaphoreSD_Restart;

// TaskHandle_t TFT_Task_Handle;
// SemaphoreHandle_t xSemaphoreTFT_Done;

// send Sound
TaskHandle_t sendSound_Task_Handle;
TaskHandle_t recv_Task_Handle;

// SemaphoreHandle_t xbufID_Mutex;


// static bool tasksCreated = false;


void checkFreeRTOS(){
#if defined(tskKERNEL_VERSION_NUMBER)
  debugOLED("FreeRTOS Version: ","Sys",true);
  debugOLED(tskKERNEL_VERSION_NUMBER,"",false);
#endif
  debugOLED("Base task number: ","",true);
  debugOLED(String(uxTaskGetNumberOfTasks()),"",false);
}

// void printTaskInfo() {
//     // Create a buffer large enough to hold the task list
//     char buffer[512];
    
//     // Print the task list to the buffer
//     vTaskList(buffer);
    
//     // Display task information
//     Serial.println("Task Name\tState\tPrio\tStack\tNum");
//     Serial.println(buffer);  // Prints: Name, State, Priority, StackHighWaterMark, Number
// }

void createSemaphore(){
  //xSemaphoreRecording_Saved = xSemaphoreCreateBinary(); 
  xSemaphoreRecording_Done = xSemaphoreCreateBinary();
  xSemaphoreHeader_Restart = xSemaphoreCreateBinary();
  // xSemaphoreSD_Done = xSemaphoreCreateBinary(); xSemaphoreSD_Restart = xSemaphoreCreateBinary(); 
  // xSemaphoreTFT_Done = xSemaphoreCreateBinary();
  // xbufID_Mutex = xSemaphoreCreateMutex();
  if (xSemaphoreHeader_Restart == NULL || xSemaphoreRecording_Done == NULL) {
   debugOLED("Failed to create semaphores!","Err",true);
    while (1);  // Stop execution if semaphore creation fails
  }
}