#pragma once
#include "setupFreeRTOS.h"

int buttonPin = 6;
int isRecording = 0;

void initButton() {
  pinMode(buttonPin, INPUT_PULLUP);
}

bool detectStatus() {
  if (!digitalRead(buttonPin)) {
    // Serial.println("pressed?");
    vTaskDelay(pdMS_TO_TICKS(50));
    if (!digitalRead(buttonPin)) {
      // Serial.println("pressed!");
      while (!digitalRead(buttonPin))
        ;
      // Serial.println("released!");
      isRecording = !isRecording;
      // Serial.println("Button!");
      // digitalWrite(LED_BUILTIN,isRecording);
      return true;
    } else {
      return false;
    }
  }
  return false;
}