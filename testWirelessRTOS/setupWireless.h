#pragma once
#include "WiFiClient.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include "setupI2S.h"

#define WIFI_SSID "mywifi"
#define WIFI_PASS "mypassword"

#define DISCOVERY_MESSAGE "WHERE_IS_MY_GIRLFRIEND"
#define EXPECTED_RESPONSE "IM_HERE"
#define END_MESSAGE "DONE"
#define TCP_READY "TCP"


// WIFI instances
WiFiServer myServer(10000);
WiFiClient myClient;

// connect
int UDP_PORT = 9998;
WiFiUDP connUDP;
IPAddress PC_IP;

// send
uint16_t OUT_PORT = 9999;
// WiFiClient BD2PC;
int fileBytesRead;
byte fileBuffer[3072];

// receive
uint16_t IN_PORT = 10000;
// WiFiServer PC2BD();
// WiFiClient recvClient;
int packetSize;
int TCP_buf_size = 3072;
char packetBuf[3072];


void initWireless() {
  // Connect to WiFi
  Serial.println("Connecting to WIFI");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    delay(500);
    Serial.println("Waiting for WIFI connection...");
  }
  Serial.println("Connected to WiFi");
  // Serial.print("IP Address: ");
  // Serial.println(WiFi.localIP());

  // Start listening for incoming UDP connections
  connUDP.begin(UDP_PORT);
}

void connectPC() {
  Serial.println("Connecting to PC");
  // TODO: add a time out; like if it doesn't receive anything for 10 sec, return
  while (1) {
    packetSize = connUDP.parsePacket();
    if (packetSize) {
      memset(packetBuf,0,255);
      connUDP.read(packetBuf, min(packetSize, sizeof(packetBuf) - 1));
      // TODO: why is the line below commented out? I think you prob want to null term
      // packetBuf[packetSize] = 0;  // null terminate

      PC_IP = connUDP.remoteIP();
      // Serial.print("Received UDP: "+String(packetBuf)+" from ");
      // Serial.println(PC_IP);

      if (strncmp(packetBuf, DISCOVERY_MESSAGE, 25) == 0) {
        // Serial.println("Discovery request received! Sending response...");
        connUDP.beginPacket(PC_IP, connUDP.remotePort());
        connUDP.write(EXPECTED_RESPONSE);
        connUDP.endPacket();
      } else if (strncmp(packetBuf, END_MESSAGE) == 0) {
        connUDP.stop();
        break;
      }

    }
  }
  Serial.println("Connected to PC IP.\n");
}

void receiveFile() {
  // TODO: i talked about this in the other files but make sure you have enough storage and handle the case when storage runs out
  createFile(reWavName,&soundFile);
  myServer.begin();
  while(1){
    myClient = myServer.available();  // Check for incoming connection
    if(myClient){Serial.println("Connected to PC, start to receive!");break;}
    // Serial.println("connecting 10000..");
    vTaskDelay(pdMS_TO_TICKS(10));
  }
  // while(myClient.connected()){
  //   myClient.write(TCP_READY);
  //   myClient.flush();
  // }
  
  while (myClient.connected()){
    // TODO: add a timeout; like add a 10 milisec delay at the end of loop
    if(myClient.available()){
      packetSize = myClient.read(packetBuf,TCP_buf_size);
      // TODO: check if the write is successful after it. write() returns num of bytes written, so check if it == packetSize
      soundFile.write((uint8_t*)packetBuf, packetSize);

      if (packetSize < TCP_buf_size) {
        packetBuf[packetSize] = '\0';  // Null-terminate the buffer
        // TODO: just a hint, rather than "25" I think you can also do strlen(END_MESSAGE)
        if (strncmp(packetBuf, END_MESSAGE, 25) == 0) {
          Serial.println("End of file reached.");
          // TODO: null terminate the file, something like packetBuf[packetSize] = '\0';
          break;  // End of file, stop receiving
        }
      }
    }
  }
  soundFile.close();
  myClient.stop();myServer.end();
  Serial.println("File received successfully.\n");
  
}

void sendFile(String fileName, File* file) {
  // open file
  if(!openFile(fileName,file)){return;}
  // // begin server to receive ready signal
  // myServer.begin();
  // while(1){
  //   myClient = myServer.available();  // Check for incoming connection
  //   if(myClient) {Serial.println("Receiving end msg..");break;}
  //   delay(10);
  // }
  // while (myClient.connected()) {
  //   if(myClient.available()){
  //     packetSize = myClient.read(packetBuf,TCP_buf_size);
  //     if (strncmp(packetBuf, END_MESSAGE, 25) == 0) {
  //       Serial.println("PC ready");
  //       break;  // End of file, start receiving
  //     }
  //   }
  // }
  // myClient.stop();myServer.end();

  // begin client
  while(1){
    // TODO: add a little time out
    if (myClient.connect(PC_IP, OUT_PORT)) {Serial.println("Connected to PC, start to send!");break;}
  }
  vTaskDelay(pdMS_TO_TICKS(10));
  // send file
  while (file->available()) {
    fileBytesRead = file->readBytes((char *)fileBuffer, sizeof(fileBuffer));
    myClient.write(fileBuffer,fileBytesRead);
    vTaskDelay(pdMS_TO_TICKS(10));
  }
  myClient.write(END_MESSAGE,sizeof(END_MESSAGE));
  
  myClient.stop();
  closeFile(file);
  Serial.println("File sent successfully.\n");
}

void sendSoundTask(void* params) {
  (void) params;
  sendFile(myWavName,&soundFile);
  vTaskDelete(NULL);
}

void recvSoundTask(void* params) {
  (void) params;
  receiveFile();
  vTaskDelete(NULL);
}
