#pragma once
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

// monitoring
// TaskHandle_t stackMonitor_Task_Handle;
TaskHandle_t manager_Task_Handle;

// recording
TaskHandle_t recording_Task_Handle;
SemaphoreHandle_t xSemaphoreRecording_Saved;
SemaphoreHandle_t xSemaphoreRecording_Done;

// displaying
TaskHandle_t header_Task_Handle;

TaskHandle_t SD_Task_Handle;
SemaphoreHandle_t xSemaphoreSD_Done;
SemaphoreHandle_t xSemaphoreSD_Restart;
SemaphoreHandle_t xSemaphoreHeader_Restart;

TaskHandle_t TFT_Task_Handle;
SemaphoreHandle_t xSemaphoreTFT_Done;

// send Sound
TaskHandle_t sendSound_Task_Handle;
TaskHandle_t recvSound_Task_Handle;

// SemaphoreHandle_t xbufID_Mutex;


// static bool tasksCreated = false;


void checkFreeRTOS(){
#if defined(tskKERNEL_VERSION_NUMBER)
  Serial.print("FreeRTOS Version: ");
  Serial.println(tskKERNEL_VERSION_NUMBER);
#endif
  Serial.print("Base task number: ");
  Serial.println(uxTaskGetNumberOfTasks());
}

void printTaskInfo() {
    // Create a buffer large enough to hold the task list
    char buffer[512];
    
    // Print the task list to the buffer
    vTaskList(buffer);
    
    // Display task information
    Serial.println("Task Name\tState\tPrio\tStack\tNum");
    Serial.println(buffer);  // Prints: Name, State, Priority, StackHighWaterMark, Number
}

void createSemaphore(){
  //xSemaphoreRecording_Saved = xSemaphoreCreateBinary(); 
  xSemaphoreRecording_Done = xSemaphoreCreateBinary();
  
  xSemaphoreSD_Done = xSemaphoreCreateBinary(); xSemaphoreSD_Restart = xSemaphoreCreateBinary(); xSemaphoreHeader_Restart = xSemaphoreCreateBinary();
  xSemaphoreTFT_Done = xSemaphoreCreateBinary();
  // xbufID_Mutex = xSemaphoreCreateMutex();
  if (xSemaphoreHeader_Restart == NULL || xSemaphoreRecording_Done == NULL || xSemaphoreSD_Done == NULL || xSemaphoreTFT_Done == NULL || xSemaphoreSD_Restart == NULL) {
    Serial.println("Failed to create semaphores!");
    while (1);  // Stop execution if semaphore creation fails
  }
}