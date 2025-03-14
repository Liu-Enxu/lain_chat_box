#pragma once

#include <SD.h>
#include "FS.h"
#include "setupSPI.h"


SDClass SD0;
SDClass SD1;

File animationFile;
File soundFile;

uint8_t MSB,LSB;
uint16_t color_val;

//init SD1
void initSD1(){
  bool init = SD1.begin(_CS1,23000000,SPI1);
  while (!init) { // 23000000
    Serial.println("initialization of bus1 SD failed!");
    delay(500);
    init = SD1.begin(_CS1,23000000,SPI1);
  }
}

// open file
void openFile(const String& str,File* file){
  // file name
  String filename = str + ".bin"; // or txr
  // const char* fileName = filename.c_str();
  // String filename = "/bad_appleASCII.bin";
  
  // check if exists
  // Serial.println("Checking file existence...");
  if (!SD1.exists(filename.c_str())) {
    Serial.println(filename+" doesn't exist.");
    return;
  } //Serial.println(filename+" exists.");
  delay(200);

  // open file
  // Serial.println("Opening file...");
  *file = SD1.open(filename.c_str(),FILE_READ);
  if (!*file) {
    Serial.println("SD Card: error on opening file "+filename);
    return;
  } //Serial.println("File opened.");
}

// close file
void closeFile(File* file){
  file->close();
}

