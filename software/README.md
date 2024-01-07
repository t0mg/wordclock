# Software

This section of the [Wordclock project](../README.md) covers the software running on the clock's ESP32.

## Features

- Supports English, French and Dutch clock faces
- Web interface over WiFi with captive portal for initial WiFi configuration
- Can use network time (NTP) or manual setting
- Interface offers various orther options such as display color, lignt sensor sensitivity

## Demo video

[Here's a video](https://youtu.be/WF_X5soabm0) showing the Wordclock and its web interface in action.

## Build with PlatformIO

This is the recommended method as it makes it easy to manage dependencies and build for a specific flavor of ESP32. If you do not want to use PlatformIO, see [next section](#build-with-arduino-ide) for detailed instructions.

1. [Install PlatformIO and VS Code](https://docs.platformio.org/en/latest/integration/ide/pioide.html)
2. Load the project in VS Code (select the folder containing `plaftformio.ini`)
3. Verify that the target board in `platformio.ini` matches your ESP32 hardware
4. Build and upload

## Build with Arduino IDE

### Add ESP32 boards

If you want to use Arduino IDE only, you need to install the [Arduino core for the ESP32](https://github.com/espressif/arduino-esp32). To do so, the preferred method is to use the Boards Manager and is detailed [here](https://github.com/espressif/arduino-esp32/blob/master/docs/arduino-ide/boards_manager.md).

Once installed, select the ESP32 board from `Tools > Board > ESP32 Arduino > ESP32 Dev Module` (or whatever is most appropriate for your specific board).

You will also need to rename the `wordclock.cpp` file to `wordclock.ino`.

_Note: you might also need to install the appropriate USB driver for your ESP32 board (for example if it has a CP210x USB to UART bridge, it's [this one](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers))._

### Add required dependencies

This project relies on several additional libraries that need to be installed in the Arduino IDE. Open the Library Manager (`Tools > Manage Libraries...`), then search an add the following:

- [RTClib](https://github.com/adafruit/RTClib) by Adafruit
- [IotWebConf](https://github.com/prampec/IotWebConf) by Balazs Kelemen **version 2.3.x** (not compatible with version 3 yet)
- [NeoPixelBus](https://github.com/Makuna/NeoPixelBus) by Michael C. Mille

You are now ready to compile and flash the `wordclock.ino` sketch.

## OTA update

After the firwmare has been flashed over USB once, you can use the OTA feature to flash further updates: build the new binary file, then open the web portal of your clock, click the `Firmware update` link at the bottom and upload the new `.bin` file.

- If you are using PatformIO, the build file is located in `.pio\build\<environment name>\firmware.bin`.
- If you are using Arduino IDE, use `Sketch > Export compiled Binary` to export the file.

## Credits

A large part of this source was based on the code from these two projects (both of which were written for the same hardware):
- https://github.com/e-noyau/wordclock (for the clockface and overall architecture)
- https://bitbucket.org/gliktaras/word-clock (for the configuration portal)
