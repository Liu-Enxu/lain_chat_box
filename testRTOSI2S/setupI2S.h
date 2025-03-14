#pragma once
#include <I2S.h>
#include "setupSD.h"
#include "setupButton.h"


// INMP441 config
#define PIN_I2S_BCLK 18 // SCK; WS needs to be Pin_BCLK+1!!!
#define PIN_I2S_DIN 11 // SD(Serial Data pin); also place-hold rx
#define PIN_I2S_MCLK 0 // Does not exist, need to pull down

// MAX98357 config
// #define POUT_I2S_BCLK 18 // SCK; WS needs to be Pin_BCLK+1!!!
// #define POUT_I2S_DIN 11 // SD(Serial Data pin); also place-hold rx
// #define POUT_I2S_MCLK 0 // Does not exist, need to pull down

// make changes as needed
#define RECORD_TIME   10  // seconds, The maximum value is 240
#define WAV_FILE_NAME "data"

// do not change for best
#define SAMPLE_RATE 16000U
#define SAMPLE_BITS 16
#define WAV_HEADER_SIZE 44
#define VOLUME_GAIN 2

I2S inI2S(INPUT);
I2S outI2S(OUTPUT);
int16_t l16=0;
int16_t r16=0;


String fileName = "/input.wav";


void initI2SIn(){
  // inI2S = I2S(INPUT,PIN_I2S_BCLK,PIN_I2S_DIN,PIN_I2S_MCLK,PIN_I2S_DIN);
  inI2S.setMCLK(PIN_I2S_MCLK);pinMode(PIN_I2S_MCLK, OUTPUT);digitalWrite(PIN_I2S_MCLK, LOW);
  
  inI2S.setDATA(PIN_I2S_DIN);
  inI2S.setBCLK(PIN_I2S_BCLK);
  
  inI2S.setBitsPerSample(SAMPLE_BITS);
  inI2S.setFrequency(SAMPLE_RATE);
  // inI2S.setSysClk(SAMPLE_RATE);

  if (!inI2S.begin()) {
    Serial.println("initialization of Mic failed!");
    while (1);
  }Serial.println("Mic initialized");
  
}

// void initI2SOut(){
//   // outI2S = I2S(INPUT,PIN_I2S_BCLK,PIN_I2S_DIN,PIN_I2S_MCLK,PIN_I2S_DIN);
//   outI2S.setMCLK(POUT_I2S_MCLK);pinMode(POUT_I2S_MCLK, OUTPUT);digitalWrite(PIN_I2S_MCLK, LOW);
  
//   outI2S.setDATA(POUT_I2S_DIN);
//   outI2S.setBCLK(POUT_I2S_BCLK);
  
//   outI2S.setBitsPerSample(SAMPLE_BITS);
//   outI2S.setFrequency(SAMPLE_RATE);
//   // outI2S.setSysClk(SAMPLE_RATE);

//   if (!outI2S.begin()) {
//     Serial.println("initialization of Mic failed!");
//     while (1);
//   }Serial.println("Mic initialized");
  
// }

void generate_wav_header(uint8_t *wav_header, uint32_t wav_size, uint32_t sample_rate)
{
  // See this for reference: http://soundfile.sapp.org/doc/WaveFormat/
  uint32_t file_size = wav_size + WAV_HEADER_SIZE - 8;
  uint32_t byte_rate = SAMPLE_RATE * SAMPLE_BITS / 8;
  const uint8_t set_wav_header[] = {
    'R', 'I', 'F', 'F', // ChunkID
    file_size, file_size >> 8, file_size >> 16, file_size >> 24, // ChunkSize
    'W', 'A', 'V', 'E', // Format
    'f', 'm', 't', ' ', // Subchunk1ID
    0x10, 0x00, 0x00, 0x00, // Subchunk1Size (16 for PCM)
    0x01, 0x00, // AudioFormat (1 for PCM)
    0x01, 0x00, // NumChannels (1 channel)
    sample_rate, sample_rate >> 8, sample_rate >> 16, sample_rate >> 24, // SampleRate
    byte_rate, byte_rate >> 8, byte_rate >> 16, byte_rate >> 24, // ByteRate
    0x02, 0x00, // BlockAlign
    0x10, 0x00, // BitsPerSample (16 bits)
    'd', 'a', 't', 'a', // Subchunk2ID
    wav_size, wav_size >> 8, wav_size >> 16, wav_size >> 24, // Subchunk2Size
  };
  memcpy(wav_header, set_wav_header, sizeof(set_wav_header));
}

void record_wav(String fileName){
  // uint32_t rec_total_size = (SAMPLE_RATE * SAMPLE_BITS / 8) * RECORD_TIME;
  uint32_t rec_buf_size = (SAMPLE_RATE * SAMPLE_BITS / 8) * 2; // suppose two 2s buffer
  uint32_t rec_size_counter = 0;

  uint32_t buffer_idx = 0;
  uint8_t *rec_buffer0 = NULL;
  
  Serial.printf("Start recording ...\n");
  soundFile = SD1.open(fileName.c_str(), FILE_WRITE);
  // Write the header to the WAV file
  uint8_t wav_header[WAV_HEADER_SIZE] = {0};
  // write place-holder first since don't know real size // 
  soundFile.write(wav_header, WAV_HEADER_SIZE);

  // PSRAM malloc for recording for ESP32; for rp2040 simply SRAM
  rec_buffer0 = (uint8_t *)malloc(rec_buf_size); memset(rec_buffer0, 0, rec_buf_size);
  if (rec_buffer0 == NULL) {Serial.printf("malloc failed! Exiting...\n");return;}

  // Start recording
  while(1){
    if (xSemaphoreTake(xSemaphoreRecording_Done, 0) == pdTRUE) {
      break;  // Exit the loop
    }

    inI2S.read16(&l16, &r16); // use left channel only!!! L/R pin pulled down       
    // int16_t test = 0x18;
    // Serial.printf("sizes: %d %d\r\n", rec_size_counter, rec_total_size); 
    
    l16 <<= VOLUME_GAIN;
    memcpy( (uint8_t *)(rec_buffer0)+buffer_idx,&l16,sizeof(l16)); // rec_buffer[buffer_flag])
    
    buffer_idx += 2;
    rec_size_counter += 2;

    if(rec_size_counter % rec_buf_size == 0){
      // Serial.printf("new buffer\n");
      if(soundFile.write(rec_buffer0, rec_buf_size) != rec_buf_size){Serial.printf("Write Failed!\n");}
      buffer_idx = 0;
    }

  }

  // Rewrite the header
  soundFile.seek(0);
  generate_wav_header(wav_header, rec_size_counter, SAMPLE_RATE);
  soundFile.write(wav_header,WAV_HEADER_SIZE);

  // End of file editing
  free(rec_buffer0);  // free(rec_buffer[0]);free(rec_buffer[1]);
  soundFile.close();
  Serial.println("Recording complete, wait for a few sec before next round");
  pdMS_TO_TICKS(1000);
}

void recordingTask(void *pvParameters){
  while(1){
    xSemaphoreTake(xSemaphoreRecording_Start, portMAX_DELAY);
    Serial.print("Checking for old file...");
    for(int i=0;i<5;i++){ // make sure to read the file. Depracate this in the future, delete wav after send
      if (SD1.exists(fileName.c_str())) {  // Check if the file exists
        SD1.remove(fileName.c_str());      // Delete the file
        while(SD1.exists(fileName.c_str())); // make sure the file is deleted fully, not sure if necessary
        Serial.println("Old file deleted.");
        break;
      }
    } 
    record_wav(fileName);
  }
}

