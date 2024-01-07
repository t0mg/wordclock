#pragma once

#include <NeoPixelAnimator.h>
#include <NeoPixelBus.h>

#include "LDRReader.h"

/* An 8-bit gamma-correction table from the Adafruit NeoPixel lib.
   Copy & paste this snippet into a Python REPL to regenerate:
import math
gamma=2.6
for x in range(256):
    print("{:3},".format(int(math.pow((x)/255.0,gamma)*255.0+0.5))),
    if x&15 == 15: print
*/
static const uint8_t PROGMEM gammaTable_[256] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,
    1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,  2,  3,  3,  3,  3,
    3,  3,  4,  4,  4,  4,  5,  5,  5,  5,  5,  6,  6,  6,  6,  7,
    7,  7,  8,  8,  8,  9,  9,  9, 10, 10, 10, 11, 11, 11, 12, 12,
   13, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19, 20,
   20, 21, 21, 22, 22, 23, 24, 24, 25, 25, 26, 27, 27, 28, 29, 29,
   30, 31, 31, 32, 33, 34, 34, 35, 36, 37, 38, 38, 39, 40, 41, 42,
   42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57,
   58, 59, 60, 61, 62, 63, 64, 65, 66, 68, 69, 70, 71, 72, 73, 75,
   76, 77, 78, 80, 81, 82, 84, 85, 86, 88, 89, 90, 92, 93, 94, 96,
   97, 99,100,102,103,105,106,108,109,111,112,114,115,117,119,120,
  122,124,125,127,129,130,132,134,136,137,139,141,143,145,146,148,
  150,152,154,156,158,160,162,164,166,168,170,172,174,176,178,180,
  182,184,186,188,191,193,195,197,199,202,204,206,209,211,213,215,
  218,220,223,225,227,230,232,235,237,240,242,245,247,250,252,255};

//
// Controls the brighness of a LED strip based on the light value of a
// LDR Reader.
//
// Create this object and then call setup() to initialize it and then invoke
// loop() as often as possible.
//
class BrightnessController
{
public:
  BrightnessController();

  void setup();
  void loop();

  void setSensorSensitivity(int value) { lightSensor_.sensitivity = value; };
  bool hasChanged()
  {
    bool res = changed_;
    changed_ = false;
    return res;
  };
  void setOriginalColor(RgbColor color) { original_ = color; }
  RgbColor getCorrectedColor() { return corrected_; };

  /*!
    @brief   A gamma-correction function for RgbColor. Makes color
             transitions appear more perceptially correct.
    @param   color RgbColor
    @return  Gamma-adjusted RgbColor.
  */
  static RgbColor gammaAdjust(RgbColor color)
  {
    return RgbColor(pgm_read_byte(&gammaTable_[color.R]),
                    pgm_read_byte(&gammaTable_[color.G]),
                    pgm_read_byte(&gammaTable_[color.B]));
  }

private:
  // The target color at maximum brihtness.
  RgbColor original_ = RgbColor(255);

  // Original color dimmed according to the current sensor reading.
  RgbColor corrected_;

  // Dirty flag.
  bool changed_;

  // The light sensor.
  LDRReader lightSensor_;
};
