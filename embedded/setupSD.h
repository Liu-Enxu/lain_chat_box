#pragma once

#include <SD.h>
#include "FS.h"
#include "setupSPI.h"
#include "setupOLED.h"
#include <vector>

SDClass SD0;
SDClass SD1;

File animationFile;
File soundFile;
std::vector<uint8_t> transcription;

uint8_t MSB,LSB;
uint16_t color_val;

//init SD1
void initSD1(){
  bool init = SD1.begin(_CS1,23000000,SPI1);
  if(!init) debugOLED("initialization of bus1 SD failed! Retry...","",true);
  while (!init) { // 23000000
    delay(500);
    init = SD1.begin(_CS1,23000000,SPI1);
  }
}

// delete file
void deleteFile(String fileName){
  for(int i=0;i<5;i++){ // make sure to read the file. Depracate this in the future, delete wav after send
    if (SD1.exists(fileName.c_str())) {  // Check if the file exists
      // Delete the file
      debugOLED("Deleting.."+fileName,"",true);
      while(SD1.exists(fileName.c_str())){
        SD1.remove(fileName.c_str());
      }; // make sure the file is deleted fully, not sure if necessary
      debugOLED("Old file deleted.","",false);
      return;
    }
  }debugOLED("No such file to delete.","",true);
}

// create file
void createFile(String fileName,File* file){

  debugOLED("Checking for old file...","",false);
  deleteFile(fileName);
  
  while(1){
    *file = SD1.open(fileName.c_str(), FILE_WRITE);
    if(*file){break;}
  }
}

// open file
bool openFile(const String& str,File* file){
  // file name
  String filename = str; // or txr
  // const char* fileName = filename.c_str();
  // String filename = "/bad_appleASCII.bin";
  
  // check if exists
  // Serial.println("Checking file existence...");
  if (!SD1.exists(filename.c_str())) {
    debugOLED(filename+" doesn't exist.","",true);
    return false;
  } //Serial.println(filename+" exists.");
  delay(200);

  // open file
  // Serial.println("Opening file...");
  *file = SD1.open(filename.c_str(),FILE_READ);
  if (!*file) {
    debugOLED("SD Card: error on opening file "+filename,"",true);
    return false;
  } //Serial.println("File opened.");

  return true;
}

// close file
void closeFile(File* file){
  file->close();
}


