#pragma once

int buttonPin = 20;
int isRecording = 0;

void initButton(){
  pinMode(buttonPin,INPUT_PULLUP);
}

bool detectStatus(){ // rewrite?
  if(!digitalRead(buttonPin)){
    // Serial.println("pressed?");
    delay(50);
    if(!digitalRead(buttonPin)){
      // Serial.println("pressed!");
      while(!digitalRead(buttonPin));
      // Serial.println("released!");
      isRecording = !isRecording;
      digitalWrite(LED_BUILTIN,isRecording);
      return true;
    } else {return false;}
  }
  return false;
}