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

// delete file
void deleteFile(String fileName){
  for(int i=0;i<5;i++){ // make sure to read the file. Depracate this in the future, delete wav after send
    if (SD1.exists(fileName.c_str())) {  // Check if the file exists
      // Delete the file
      Serial.println("Deleting..");
      while(SD1.exists(fileName.c_str())){
        SD1.remove(fileName.c_str());
      }; // make sure the file is deleted fully, not sure if necessary
      Serial.println("Old file deleted.");
      return;
    }
  }Serial.println("No such file to delete.");
}

// create file
void createFile(String fileName,File* file){

  Serial.print("Checking for old file...");
  deleteFile(fileName);

  // TODO: I am not sure about this while(1). could this cause an infinitely loop? I see that you check for file existence in deleteFile() but not here. What if file doesn't exist? What if the file is too long and you run out of storage?
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
    Serial.println(filename+" doesn't exist.");
    return false;
  } //Serial.println(filename+" exists.");
  delay(200);

  // open file
  // Serial.println("Opening file...");
  *file = SD1.open(filename.c_str(),FILE_READ);
  if (!*file) {
    Serial.println("SD Card: error on opening file "+filename);
    return false;
  } //Serial.println("File opened.");

  return true;
}

// close file
void closeFile(File* file){
  file->close();
}


