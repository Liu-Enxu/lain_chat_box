# lain_chat_box (ongoing project)
An AI chat bot with a TFT screen showing the character status.

Uses 2SPI bus, one communicates with TFT LCD; one reads from SD card

Achieved ~30 fps refresh rate of the TFT screen by using FreeRTOS, DMA & overclocking

Use I2S microphone to record audio

https://github.com/user-attachments/assets/507d36e9-b06a-48fc-ae82-5705394fe05f

BOM:
1. Raspberry Pi Pico W
2. ST7735 TFT LCD screen
3. SD module
4. INMP441
5. MAX98357

TODO:

HARDWARE
1. Photo interface on the embedded system
2. Add buttons built with I2C screens on hardware as user interface, symbol on the screen changes with dialog content

SOFTWARE
1. Connect to local server/pc for AI processing
2. Dynamically generate character video using generative model, based on the dialog content
3. Utilize Ollama, Whisper, and Coqui TTS

----

### Installation on PC/Server: 
Clone the git repo. Conda is suggested to use for setting up the environemnt: run `conda env create -f environment.yml`