#pragma once
#include <Wire.h>


#define SDA1 14
#define SCL1 15
#define SDA0 16
#define SCL0 17
#define I2C_CLOCK 100000 // 100k

#define MUX_addr 0x70

void initI2C1(){
  Wire1.setSDA(SDA1);
  Wire1.setSCL(SCL1);
  Wire1.setClock(I2C_CLOCK);
  Wire1.begin();
  // Wire1.end();
}

void initI2C0(){
  Wire.setSDA(SDA0);
  Wire.setSCL(SCL0);
  Wire.setClock(I2C_CLOCK);
  Wire.begin();
  // Wire.end();
}

void select_MUX(uint8_t port){
  if (port>7) return;
  Wire.beginTransmission(MUX_addr);
  Wire.write(1<<port);
  Wire.endTransmission();
}