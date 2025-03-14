# lain_chat_box (ongoing project)
An AI chat bot with a TFT screen showing the character status.

Uses 2SPI bus, one communicates with TFT LCD; one reads from SD card

Achieved ~30 fps refresh rate of the TFT screen by using FreeRTOS, DMA & overclocking

Use I2S microphone to record audio

https://github.com/user-attachments/assets/507d36e9-b06a-48fc-ae82-5705394fe05f

BOM:
Raspberry Pi Pico W
ST7735 TFT LCD screen
SD module
INMP441 
MAX98357

TODO:
HARDWARE
1. Photo interface on the embedded system
2. Add buttons built with I2C screens on hardware as user interface, symbol on the screen changes with dialog content
SOFTWARE
1. Connect to local server/http listener on pc
2. Dynamically generate character video using generative model, based on the dialog content
