#pragma once
#include <SPI.h>
#include <Arduino.h>

#define _CS0 17
#define _SCK0 18
#define _MOSI0 19
#define _MISO0 16

#define _CS1 13
#define _SCK1 10
#define _MOSI1 11
#define _MISO1 12

void initSPI1(){
  SPI1.setCS(_CS1);
  SPI1.setRX(_MISO1);
  SPI1.setTX(_MOSI1);
  SPI1.setSCK(_SCK1);
  pinMode(_CS1,OUTPUT);
  SPI1.begin(true);
}

void initSPI0(){
  SPI.setCS(_CS0);
  SPI.setRX(_MISO0);
  SPI.setTX(_MOSI0);
  SPI.setSCK(_SCK0);
  pinMode(_CS0,OUTPUT);
  SPI.begin(true);
}