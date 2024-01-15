#include "BrightnessController.h"

#define MIN_DIM 100

BrightnessController::BrightnessController() {}

void BrightnessController::setOriginalColor(RgbColor color)
{
  original_ = color;
  corrected_ = gammaAdjust(original_);
  changed_ = true;
}

void BrightnessController::setup()
{
  lightSensor_.setup();
  corrected_ = gammaAdjust(original_);
}

void BrightnessController::loop()
{
  lightSensor_.loop();

  if (lightSensor_.sensitivity == 0)
  {
    RgbColor gamma_corrected = gammaAdjust(original_);
    changed_ = corrected_ != gamma_corrected;
    corrected_ = gamma_corrected;
    return;
  }

  float dim = (255 - MIN_DIM) * lightSensor_.reading() + MIN_DIM;
  RgbColor newColor = gammaAdjust(original_.Dim(static_cast<int>(dim)));

  if (abs(corrected_.CalculateBrightness() - newColor.CalculateBrightness()) < 5.)
  {
    return; // don't adjust for small changes.
  }

  corrected_ = newColor;
  changed_ = true;
}
