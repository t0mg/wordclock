#pragma once
#include "nodo.h"

/*
 * Light sensor. Reads the ambient light and build a representative value
 * between 0.0 and 1.0. Smooth the reading by doing a moving average.
 */
class LDRReader
{
public:
  // Reaction speed must be greater than 0 and at most 1.0. This controls the
  // input smoothing, the higher the number the less smooth the data will be.
  // The pin number points to the pin the data line is soldered on.
#ifndef LDR_PIN
#define LDR_PIN 33
#endif
  LDRReader(int pinNumber = LDR_PIN, float reactionSpeed = .1, int sensitivity = 1);

  // Must be called from the ino setup and loop.
  void setup();
  void loop();

  // Returns a value between 0. (no light) and 1. (much lights)
  float reading();

  int sensitivity;

private:
  float _currentLDR;
  float _reactionSpeed;
  int _pin;
};
