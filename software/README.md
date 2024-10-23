# Software

This section of the [Wordclock project](../README.md) covers the software running on the clock's ESP32. This page covers the basic features and how to build and deploy from source.

For detailed usage instructions, please refer to the [User's Manual](UsersManual.md).

## Features

- Supports English, French, Italian and Dutch clock faces
- Web interface over WiFi with captive portal for initial WiFi configuration (see [User's manual](UsersManual.md) for details)
- Can use network time (NTP) or manual setting
- Interface offers various other options such as display color, light sensor sensitivity, and [OTA firmware update](#ota-update)
- [MQTT client](UsersManual.md#mqtt-client) to control the clock via home automation platforms such as [Home Assistant](https://www.home-assistant.io/)
- Go beyond time display with [text scrolling](UsersManual.md#text-scroller) and an experimental web interface for real-time [painting](UsersManual.md#paint)


## Demo video

[Here's a video](https://youtu.be/WF_X5soabm0) showing the Wordclock with 1.0 firmware and its web interface in action. New features were added since, such as MQTT and Paint.

## Flashing over USB with the Wordclock Flasher utility

[Wordclock Flasher](https://t0mg.github.io/wordclock-flasher/) is a web based companion tool that can be used to flash the Wordclock over USB with a prebuilt release firmware.

If you have not flashed your clock before, it is the fastest and easiest way to get there. Once you have flashed your device for the first time, the recommended path for upgrading is to use [OTA updates](#ota-update).

### Wordclock Flasher instructions
- Head over to the [Wordclock Flasher website](https://t0mg.github.io/wordclock-flasher/) using Chrome, Edge or another [Web Serial compatible browser](https://caniuse.com/web-serial)
- Press the `CONNECT` button, a system dialog appears to select a serial interface
- Connect your Wordclock to your computer over USB and a new entry should show up in the dialog. Select that and confirm
- The interface should identify your clock based on the chip it uses ([original DIY project](../README.md#hardware), or Nodo kit from [nodo-shop.nl](https://www.nodo-shop.nl/en/52-wordclock)) and display a list of available firmware files
- Click `FLASH`

## Release binaries

If you are not interested in modifying the source code, you can find prebuilt binaries [here](https://github.com/t0mg/wordclock/releases) that can be flashed on your ESP32 with [esptool.py](https://docs.espressif.com/projects/esptool/en/latest/esp32/esptool/flashing-firmware.html) or even directly from Chrome browser with [esptool-js](https://espressif.github.io/esptool-js/)

Use the `firmware_esp32dev` files if you have a custom built clock based on the ESP32 Dev board, or the `firmware_nodo` files if you purchased a [nodo clock kit](https://www.nodo-shop.nl/en/52-wordclock) (which is powered by an ESP32-C3 chip and has the LEDs laid out in a different order).

## Build with PlatformIO

This is the recommended method as it makes it easy to manage dependencies and build for a specific flavor of ESP32. If you do not want to use PlatformIO, see [below](#deprecated-build-with-arduino-ide) for detailed instructions.

1. [Install PlatformIO and VS Code](https://docs.platformio.org/en/latest/integration/ide/pioide.html)
2. Load the project in VS Code (select the folder containing `platformio.ini`)
3. Verify that the target board in `platformio.ini` matches your ESP32 hardware
4. Build and upload

**Important**: Platform IO builds and flashes multiple bin files (bootloader, partition, firmware). To merge those into a single binary for flashing separately from e.g. esptool or the OTA updater, run `pio run -t mergebin` from a PlatformIO console and look for the file ending with "merged.bin" in the `.pio/build` folder.

## OTA update

After the firwmare has been flashed over USB once, you can use the OTA feature to flash further updates: build the new binary file, then open the web portal of your clock, click the `Firmware update` link at the bottom and upload the new `firmware.bin` file. For OTA we don't use the "merged" version mentioned in the previous section.

- Pre-built releases can be found [here](https://github.com/t0mg/wordclock/releases).
- In PatformIO, the build file is located in `.pio\build\<environment name>\firmware.bin`.
- If you are building with Arduino IDE, use `Sketch > Export compiled Binary` to export the file.

__Warning__: if the `firmware config version` displayed at the very bottom of the web interface changes, your settings will be reset. They should otherwise remain.

## [Deprecated] Build with Arduino IDE

_**Note:** this project now relies on PlatformIO features such as board-specific configurations and [binary data embedding](https://docs.platformio.org/en/latest/platforms/espressif32.html#embedding-binary-data), and will therefore **require changes in the code and project files to build with the Arduino IDE**, which is why this section is considered deprecated._

### Add ESP32 boards

If you want to use Arduino IDE only, you need to install the [Arduino core for the ESP32](https://github.com/espressif/arduino-esp32). To do so, the preferred method is to use the Boards Manager and is detailed [here](https://github.com/espressif/arduino-esp32/blob/master/docs/arduino-ide/boards_manager.md).

Once installed, select the ESP32 board from `Tools > Board > ESP32 Arduino > ESP32 Dev Module` (or whatever is most appropriate for your specific board).

You will also need to rename the `wordclock.cpp` file to `wordclock.ino`.

_Note: you might also need to install the appropriate USB driver for your ESP32 board (for example if it has a CP210x USB to UART bridge, it's [this one](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers))._

### Add required dependencies

This project relies on several additional libraries that need to be installed in the Arduino IDE. Open the Library Manager (`Tools > Manage Libraries...`), then search an add the following:

- [RTClib](https://github.com/adafruit/RTClib) by Adafruit
- [IotWebConf](https://github.com/prampec/IotWebConf) by Balazs Kelemen
- [NeoPixelBus](https://github.com/Makuna/NeoPixelBus) by Michael C. Mille
- [arduino-mqtt](https://github.com/256dpi/arduino-mqtt) by Joël Gähwiler

You are now ready to compile and flash the `wordclock.ino` sketch.

## Credits

A large part of this source was based on the code from these two projects (both of which were written for the same hardware) with their explicit permission:
- https://github.com/e-noyau/wordclock (for the clockface and overall architecture)
- https://bitbucket.org/gliktaras/word-clock (for the configuration portal)
