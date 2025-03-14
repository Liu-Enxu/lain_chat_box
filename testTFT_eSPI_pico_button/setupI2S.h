#pragma once
#include <I2S.h>
#include "setupSD.h"
#include "setupButton.h"


// INMP441 config
#define PIN_I2S_BCLK 18 // SCK; WS needs to be Pin_BCLK+1!!!
#define PIN_I2S_DIN 11 // SD(Serial Data pin); also place-hold rx
#define PIN_I2S_MCLK 0 // Does not exist, need to pull down

// MAX98357 config
#define POUT_I2S_BCLK 18 // SCK; WS needs to be Pin_BCLK+1!!!
#define POUT_I2S_DIN 11 // SD(Serial Data pin); also place-hold rx
#define POUT_I2S_MCLK 0 // Does not exist, need to pull down

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

void initI2SOut(){
  // outI2S = I2S(INPUT,PIN_I2S_BCLK,PIN_I2S_DIN,PIN_I2S_MCLK,PIN_I2S_DIN);
  outI2S.setMCLK(PIN_I2S_MCLK);pinMode(PIN_I2S_MCLK, OUTPUT);digitalWrite(PIN_I2S_MCLK, LOW);
  
  outI2S.setDATA(PIN_I2S_DIN);
  outI2S.setBCLK(PIN_I2S_BCLK);
  
  outI2S.setBitsPerSample(SAMPLE_BITS);
  outI2S.setFrequency(SAMPLE_RATE);
  // outI2S.setSysClk(SAMPLE_RATE);

  if (!outI2S.begin()) {
    Serial.println("initialization of Mic failed!");
    while (1);
  }Serial.println("Mic initialized");
  
}

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
  uint32_t rec_total_size = (SAMPLE_RATE * SAMPLE_BITS / 8) * RECORD_TIME;
  uint32_t rec_buf_size = (SAMPLE_RATE * SAMPLE_BITS / 8) * int(RECORD_TIME/2); // suppose two 2s buffer
  uint32_t rec_size_counter = 0;

  uint32_t buffer_idx = 0;
  uint8_t *rec_buffer0 = NULL;
  // uint8_t *rec_buffer1 = NULL;
  // uint8_t *rec_buffer[2] = {NULL,NULL};
  // uint8_t buffer_flag = 0;
  
  Serial.printf("Start recording ...\n");
  soundFile = SD1.open(fileName.c_str(), FILE_WRITE);
  // Write the header to the WAV file
  uint8_t wav_header[WAV_HEADER_SIZE];
  generate_wav_header(wav_header, rec_total_size, SAMPLE_RATE);
  soundFile.write(wav_header, WAV_HEADER_SIZE);

  // PSRAM malloc for recording for ESP32; for rp2040 simply SRAM
  rec_buffer0 = (uint8_t *)malloc(rec_buf_size); memset(rec_buffer0, 0, rec_buf_size);
  if (rec_buffer0 == NULL) {
    Serial.printf("malloc failed! Exiting...\n");
    // while(1) ;
    return;
  }
  // rec_buffer[0] = (uint8_t *)malloc(rec_buf_size); memset(rec_buffer[0], 0, rec_buf_size);
  // rec_buffer[1] = (uint8_t *)malloc(rec_buf_size); memset(rec_buffer[1], 0, rec_buf_size);
  // if (rec_buffer[0] == NULL || rec_buffer[1] == NULL) {
  //   Serial.printf("malloc failed! Exiting...\n");
  //   // while(1) ;
  //   return;
  // }

  // Start recording
  // Serial.printf("sizes: %d %d\r\n", rec_buf_size, rec_total_size); 
  while(rec_size_counter<rec_total_size){
    
    if (xSemaphoreTake(xSemaphoreRecording_Done, 0) == pdTRUE) {
      // Serial.println("Restarting SDTask...");
      closeFile(&animationFile);  // Close file safely
      break;  // Exit the loop and restart reading with new gif_name
    }

    inI2S.read16(&l16, &r16); // use left channel only!!! L/R pin pulled down
    // Serial.printf("%d %d\r\n", l16, r16); 
    // Serial.printf("%d \r\n",l16);
    
    // MSB_rec = (uint8_t)((l16 >> 8) & 0xFF);
    // LSB_rec = (uint8_t)(l16 & 0xFF);
    
    // Serial.printf("sizes: %d %d\r\n", rec_size_counter, rec_total_size); 
    
    
    l16 <<= VOLUME_GAIN;
    // int16_t test = 0x18;
    memcpy( (uint8_t *)(rec_buffer0)+buffer_idx,&l16,sizeof(l16)); // rec_buffer[buffer_flag])
    
    buffer_idx += 2;
    rec_size_counter += 2;

    if(rec_size_counter>=rec_total_size || rec_size_counter % rec_buf_size == 0){
      // Serial.printf("new buffer\n");
      
      if(soundFile.write(rec_buffer0, rec_buf_size) != rec_buf_size){ //rec_buffer[buffer_flag]
        Serial.printf("Write Failed!\n");
      }
      buffer_idx = 0;
      // buffer_flag = !buffer_flag;
    }

  }

  

  // Write data to the WAV file
  // Serial.printf("Writing to the file ...\n");
  // if (file.write(rec_buffer[0], rec_buf_size) != rec_buf_size)
  //   Serial.printf("Write file Failed!\n");
  // if (file.write(rec_buffer[1], rec_buf_size) != rec_buf_size)
  //   Serial.printf("Write file Failed!\n");

  free(rec_buffer0);  // free(rec_buffer[0]);free(rec_buffer[1]);
  soundFile.close();
  Serial.printf("Recording complete: \n");
  Serial.printf("Send rec for a new sample or enter a new label\n\n");

  // Serial.println("Reading");
  // soundFile = SD1.open(fileName.c_str());
  // if (soundFile) {
  //   Serial.println("Content:");

  //   // read from the soundFile until there's nothing else in it:
  //   uint8_t byteValue;
  //   while (soundFile.available()) {
  //     // Serial.println(soundFile.read());
  //     soundFile.read(&byteValue, 1); // Reads one byte into byteValue
  //     if(byteValue) {
  //       Serial.printf("Byte: %02X\n", byteValue);
  //     }
  //   }
  //   // close the soundFile:
  //   soundFile.close();
  // } else {
  //   // if the soundFile didn't open, print an error:
  //   Serial.println("error opening");
  // }
}

void recordingTask(void *pvParameters){
  (void) pvParameters;
  String fileName = "/input.wav";
  if (SD1.exists(fileName.c_str())) {  // Check if the file exists
    SD1.remove(fileName.c_str());      // Delete the file
    Serial.println("Old file deleted.");
  }
  record_wav(fileName);
  delay(500); // delay to avoid recording multiple files at once
  vTaskDelete(NULL);
  recording_Task_Handle = NULL;
}

