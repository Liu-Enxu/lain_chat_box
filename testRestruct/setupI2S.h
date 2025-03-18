#pragma once
#include <I2S.h>
#include "setupSD.h"
#include "setupButton.h"


// INMP441 config
#define PIN_I2S_BCLK 18 // SCK; WS needs to be Pin_BCLK+1!!!
#define PIN_I2S_DIN 11 // SD(Serial Data pin); also place-hold rx
#define PIN_I2S_MCLK 0 // Does not exist, need to pull down

// MAX98357 config
#define POUT_I2S_BCLK 20 // SCK; LRC needs to be Pin_BCLK+1!!!
#define POUT_I2S_DIN 10 // SD(Serial Data pin); also place-hold rx
#define POUT_I2S_MCLK 0 // Does not exist, need to pull down

// make changes as needed
#define RECORD_TIME   10  // seconds, The maximum value is 240
#define WAV_FILE_NAME "data"

// do not change for best
#define SAMPLE_RATE 16000
#define SAMPLE_BITS 16
#define WAV_HEADER_SIZE 44
#define VOLUME_GAIN 2

I2S inI2S(INPUT);
I2S outI2S(OUTPUT);
int shutDownPin = 16;

String fileName = "/input.wav";

int16_t l16=0;
int16_t r16=0;

const uint32_t rec_buf_size = 32000; // (SAMPLE_RATE * SAMPLE_BITS / 8) * 2; 2 seconds = 64000
uint32_t rec_size_counter = 0;
uint32_t buffer_idx = 0;
uint8_t rec_buffer0[rec_buf_size] = {0};
uint8_t wav_header[WAV_HEADER_SIZE] = {0};

void initI2SIn(){
  // inI2S = I2S(INPUT,PIN_I2S_BCLK,PIN_I2S_DIN,PIN_I2S_MCLK,PIN_I2S_DIN);
  inI2S.setMCLK(PIN_I2S_MCLK);pinMode(PIN_I2S_MCLK, OUTPUT);digitalWrite(PIN_I2S_MCLK, LOW);
  
  inI2S.setDATA(PIN_I2S_DIN);
  inI2S.setBCLK(PIN_I2S_BCLK);
  
  inI2S.setBitsPerSample(SAMPLE_BITS);
  inI2S.setFrequency(SAMPLE_RATE);
  // inI2S.setSysClk(SAMPLE_RATE);

  Serial.println("test");
  if (!inI2S.begin()) {
    Serial.println("initialization of Mic failed!");
    while (1);
  }Serial.println("Mic initialized");
  
}

void initI2SOut(){
  // outI2S = I2S(INPUT,PIN_I2S_BCLK,PIN_I2S_DIN,PIN_I2S_MCLK,PIN_I2S_DIN);
  outI2S.setMCLK(POUT_I2S_MCLK);pinMode(POUT_I2S_MCLK, OUTPUT);digitalWrite(POUT_I2S_MCLK, LOW);
  
  outI2S.setDATA(POUT_I2S_DIN);
  outI2S.setBCLK(POUT_I2S_BCLK);
  
  outI2S.setBitsPerSample(SAMPLE_BITS);
  outI2S.setFrequency(SAMPLE_RATE);
  // outI2S.setSysClk(SAMPLE_RATE);

  if (!outI2S.begin()) {
    Serial.println("initialization of Amplifier failed!");
    while (1);
  }Serial.println("Amplifier initialized");
  
  outI2S.end();
  digitalWrite(shutDownPin,0);
  
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

void open_wav(String fileName){
  rec_size_counter = 0;
  buffer_idx = 0;
  
  // checking old file
  Serial.print("Checking for old file...");
  deleteFile(fileName);

  // open new file and init
  soundFile = SD1.open(fileName.c_str(), FILE_WRITE);
  // // Write the header to the WAV file
  memset(wav_header, 0, WAV_HEADER_SIZE);
  // // write place-holder first since don't know real size // 
  soundFile.write(wav_header, WAV_HEADER_SIZE);
  // // PSRAM malloc for recording for ESP32; for rp2040 simply SRAM
  // memset(rec_buffer0, 0, rec_buf_size);
}

void close_wav(){
  // Serial.println("Exitted!");
  // Rewrite the header
  soundFile.seek(0);
  generate_wav_header(wav_header, rec_size_counter, SAMPLE_RATE);
  soundFile.write(wav_header,WAV_HEADER_SIZE);
  digitalWrite(LED_BUILTIN,0);

  // End of file editing
  soundFile.close();
  // Serial.println("Recording complete, wait for a few sec before next round");
}

void play_wav(String fileName){
  inI2S.end();
  outI2S.begin();
  digitalWrite(shutDownPin,1);

  openFile(fileName,&soundFile);
  soundFile.seek(44);
  while(soundFile.available()){
    int16_t sample = soundFile.read() | (soundFile.read() << 8);  // Read 2 bytes and combine them into a 16-bit sample
    outI2S.write(sample);  // Write the sample to I2S
    vTaskDelay(pdMS_TO_TICKS(52));
  }
  soundFile.close();

  digitalWrite(shutDownPin,0);
  outI2S.end();
  inI2S.begin();
}

void recordingTask(void *pvParameters){
  
  open_wav(fileName);
  // Start recording
  digitalWrite(LED_BUILTIN,1);
  // Serial.printf("Start recording ...\n");
  while(1){
    if (xSemaphoreTake(xSemaphoreRecording_Done, 0) == pdTRUE) {
      // Serial.println("Stop recording!");
      close_wav();
      digitalWrite(LED_BUILTIN,0);
      xSemaphoreGive(xSemaphoreRecording_Saved);
      vTaskDelete(NULL);
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
  }// Exited
    
  
}

