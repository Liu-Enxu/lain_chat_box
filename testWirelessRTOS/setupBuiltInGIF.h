#pragma once
#include "gifs/listen.h"
#include "gifs/idle.h"
#include "gifs/speak.h"
#include "gifs/think.h"
#include "gifs/search.h"

typedef struct {
    uint8_t framesNum;
    uint8_t frameDelay;
    const unsigned short (*frames)[16384];  // Pointer to 2D array of frames
} flashedGifPara;

flashedGifPara idle_gifPara = {
  .framesNum = idle_framesNum,
  .frameDelay = 100,
  .frames = idle_Frames
};

flashedGifPara listen_gifPara = {
  .framesNum = listen_framesNum,
  .frameDelay = 30,
  .frames = listen_Frames
};

flashedGifPara speak_gifPara = {
  .framesNum = speak_framesNum,
  .frameDelay = 30,
  .frames = speak_Frames
};

flashedGifPara think_gifPara = {
  .framesNum = think_framesNum,
  .frameDelay = 60,
  .frames = think_Frames
};

flashedGifPara search_gifPara = {
  .framesNum = search_framesNum,
  .frameDelay = 30,
  .frames = search_Frames
};