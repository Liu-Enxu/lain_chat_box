#pragma once
#include "WiFiClient.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include "setupI2S.h"
#include "setupOLED.h"

#define WIFI_SSID "MOTOEE8E"
#define WIFI_PASS "z3887u85fn"

#define DISCOVERY_MESSAGE "WHERE_IS_MY_GIRLFRIEND"
#define EXPECTED_RESPONSE "IM_HERE"

#define TEXT_MESSAGE "TEXT"
#define AUDIO_MESSAGE "AUDIO"
#define TEXT_END_MESSAGE "TEXTEND"
#define AUDIO_END_MESSAGE "AUDIOEND"

#define END_MESSAGE "DONE"

bool recv_audio = false;
bool recv_text = false;

// WIFI instances
WiFiServer myServer(10000);
WiFiClient myClient;

// connect
int UDP_PORT = 9998;
WiFiUDP connUDP;
IPAddress PC_IP;

// send
uint16_t OUT_PORT = 9999;
int fileBytesRead;
byte fileBuffer[3072];

// receive
uint16_t IN_PORT = 10000;
int packetSize;
int TCP_buf_size = 3072;
char packetBuf[3072];

void connectWIFI(){
  while (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    delay(500);
    debugOLED("Waiting for WIFI connection...","",true);
  }
  debugOLED("Connected to WiFi","",false);
  // Serial.print("IP Address: ");
  // Serial.println(WiFi.localIP());
}

void initWireless() {
  // Connect to WiFi
  debugOLED("Connecting to WIFI...","",true);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  connectWIFI();
  // Start listening for incoming UDP connections
  connUDP.begin(UDP_PORT);
}

void connectPC() {
  debugOLED("Connecting to PC...","",true);
  while (1) {
    packetSize = connUDP.parsePacket();
    if (packetSize) {
      memset(packetBuf,0,packetSize);
      connUDP.read(packetBuf, min(packetSize, sizeof(packetBuf)/sizeof(packetBuf[0]) - 1));
      // packetBuf[packetSize] = '\0';  // null terminate

      PC_IP = connUDP.remoteIP();
      // Serial.print("Received UDP: "+String(packetBuf));
      // Serial.println(PC_IP);

      if (strncmp(packetBuf, DISCOVERY_MESSAGE,packetSize) == 0) {
        // Serial.println("Discovery request received! Sending response...");
        connUDP.beginPacket(PC_IP, connUDP.remotePort());
        connUDP.write(EXPECTED_RESPONSE);
        connUDP.endPacket();
      } else if (strncmp(packetBuf, END_MESSAGE,packetSize) == 0) {
        connUDP.stop();
        break;
      }

    }
  }
 debugOLED("Connected to PC IP.","",false);
}

void receiveFile() {
  if(WiFi.status() != WL_CONNECTED){debugOLED("WIFI lost!","Sys",true);connectWIFI();}
  createFile(reWavName,&soundFile);
  myServer.begin();
  while(1){
    myClient = myServer.available();  // Check for incoming connection
    if(myClient){debugOLED("Connected to PC, start to receive!","Sys",true);break;}
    // Serial.println("connecting 10000..");
    vTaskDelay(pdMS_TO_TICKS(10));
  }
  
  while (myClient.connected()){
    if(myClient.available()){//watchdog_update();
      packetSize = myClient.read(packetBuf,TCP_buf_size);
      soundFile.write((uint8_t*)packetBuf, packetSize);

      if (packetSize < TCP_buf_size) {
        packetBuf[packetSize] = '\0';  // Null-terminate the buffer
        if (strncmp(packetBuf, END_MESSAGE, packetSize) == 0) {
          debugOLED("End of file reached.","Sys",true);
          break;  // End of file, stop receiving
        }
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
  soundFile.close();
  myClient.stop();myServer.end();
  debugOLED("File received successfully.","Sys",true);
  
}

void receive() {
  if(WiFi.status() != WL_CONNECTED){debugOLED("WIFI lost!","Sys",true);connectWIFI();}
  // connect
  myServer.begin();
  while(1){
    myClient = myServer.available();  // Check for incoming connection
    if(myClient){debugOLED("Connected to PC, start to receive!","Sys",true);break;}
    // Serial.println("connecting 10000..");
    vTaskDelay(pdMS_TO_TICKS(10));
  }
  // recv
  while (myClient.connected()){
    if(myClient.available()){//watchdog_update();
      packetSize = myClient.read(packetBuf,TCP_buf_size);
      if(!packetSize) continue;
      // check special MSG
      if (packetSize < TCP_buf_size) {
        packetBuf[packetSize] = '\0';  // Null-terminate the buffer
        
        if (strncmp(packetBuf, END_MESSAGE, packetSize) == 0) {
          debugOLED("End of recv transmission.","",true);
          recv_audio = false;recv_text = false;
          debugOLED(String((const char*)transcription.data()),"User",true);
          break;  // End of file, stop receiving
        } else if(strncmp(packetBuf, TEXT_MESSAGE, packetSize) == 0){
          debugOLED("Receiving text...","",true);
          transcription.clear();
          recv_text = true;
          continue;
        } else if(strncmp(packetBuf, TEXT_END_MESSAGE, packetSize) == 0){
          debugOLED("text received.","",true);
          recv_text = false;
          continue;
        } else if(strncmp(packetBuf, AUDIO_MESSAGE, packetSize) == 0){
          debugOLED("Receiving audio...","",true);
          createFile(reWavName,&soundFile);
          recv_audio = true;
          continue;
        } else if(strncmp(packetBuf, AUDIO_END_MESSAGE, packetSize) == 0){
          debugOLED("Audio received.","",false);
          soundFile.close();
          recv_audio = false;
          continue;
        }
      }
      // write file
      if(recv_audio) {
        soundFile.write((uint8_t*)packetBuf, packetSize);
      } else if(recv_text){
        for (int i = 0; i < packetSize; i++) {
          transcription.push_back(packetBuf[i]);
        }
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
  
  myClient.stop();myServer.end();
  debugOLED("Transmission ends.","Sys",true);
  
}

void sendFile(String fileName, File* file) {
  if(WiFi.status() != WL_CONNECTED){debugOLED("WIFI lost!","Sys",true);connectWIFI();}
  // open file
  debugOLED("Trying to open file before send..","",true);
  if(!openFile(fileName,file)){debugOLED("Target file for send not found!","",false);return;} // should while(1)
  debugOLED("File opened, sending..","",false);

  // begin client
  // while(1){
  //   if (myClient.connect(PC_IP, OUT_PORT)) {
  //     Serial.println("Connected to PC, start to send!");
  //     break;
  //   }
  // }
  // vTaskDelay(pdMS_TO_TICKS(10));

  debugOLED("Connecting to PC...","Sys",true);
  while (!myClient.connect(PC_IP, OUT_PORT)) {
    vTaskDelay(pdMS_TO_TICKS(10));  // Yield while waiting for connection
  }
  debugOLED("Connected to PC, start to send!","",false);

  // send file
  while (file->available()) {
    // watchdog_update();
    fileBytesRead = file->readBytes((char *)fileBuffer, sizeof(fileBuffer));
    myClient.write(fileBuffer,fileBytesRead);
    vTaskDelay(pdMS_TO_TICKS(10));
  }
  myClient.write(END_MESSAGE,sizeof(END_MESSAGE));
  
  myClient.stop();
  closeFile(file);
  debugOLED("File sent successfully.","Sys",true);
}

void sendSoundTask(void* params) {
  (void) params;
  sendFile(myWavName,&soundFile);
  vTaskDelete(NULL);
}

void recvTask(void* params) {
  (void) params;
  receive();
  vTaskDelete(NULL);
}