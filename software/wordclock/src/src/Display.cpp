#include "logging.h"

#include "Display.h"
#include "ClockFace.h"

Display::Display(ClockFace* clockFace, uint8_t pin)
    : _clockFace(clockFace),
      _pixels(ClockFace::pixelCount(), pin),
      _animations(ClockFace::pixelCount(), NEO_CENTISECONDS),
      _bootAnimations(2) {}

void Display::setup()
{
  _pixels.Begin();
  _brightnessController.setup();
}

void Display::loop()
{
  if (_bootAnimations.IsAnimating()) {
    _bootAnimations.UpdateAnimations();
  } else {
    _animations.UpdateAnimations();
    if (!_off && _brightnessController.hasChanged())
    {
      DLOGLN("Brightness has changed, updating");
      _update(30); // Update in 300 ms
    }
    _brightnessController.loop();
  }
  _pixels.Show();
}

void Display::setClockFace(ClockFace* clockface)
{
  DLOGLN("Updating clockface");
  _clockFace = clockface;
  _update();
}

void Display::setColor(const RgbColor &color)
{
  DLOGLN("Updating color");
  _brightnessController.setOriginalColor(color);
}

void Display::setOff()
{
  _off = true;
  _update();
}

void Display::setOn()
{
  _off = false;
  _update();
}

void Display::_update(int animationSpeed)
{
  DLOGLN("Updating display");

  _animations.StopAll();
  static const RgbColor black = RgbColor(0x00, 0x00, 0x00);

  // For all the LED animate a change from the current visible state to the new
  // one.
  const std::vector<bool> &state = _clockFace->getState();

  for (int index = 0; index < state.size(); index++)
  {
    RgbColor originalColor = _pixels.GetPixelColor(index);
    RgbColor targetColor = _off ? black : (state[index] ? _brightnessController.getCorrectedColor() : black);

    AnimUpdateCallback animUpdate = [=](const AnimationParam &param) {
      float progress = NeoEase::QuadraticIn(param.progress);
      RgbColor updatedColor = RgbColor::LinearBlend(
          originalColor, targetColor, progress);
      _pixels.SetPixelColor(index, updatedColor);
    };
    _animations.StartAnimation(index, animationSpeed, animUpdate);
  }
}

void Display::updateForTime(int hour, int minute, int second, int animationSpeed)
{

  if (_bootAnimations.IsAnimating() || !_clockFace->stateForTime(hour, minute, second, _show_ampm))
  {
    return; // Nothing to update.
  }

  DLOG("Time: ");
  DLOG(hour);
  DLOG(":");
  DLOGLN(minute);

  _update(animationSpeed);
}

void Display::_circle(uint16_t x, uint16_t y, int radius, RgbColor color)
{
  for (int i = 0; i < 11; i++)
  {
    for (int j = 0; j < 10; j++) {
      double distance = _distance(i, j, x, y);
      if (distance < radius && distance > radius - 2)
      {
        _pixels.SetPixelColor(_clockFace->map(i, j), color);
      }
    }
  }
}

void Display::_colorCornerPixels(RgbColor color)
{
  for (int corner = _clockFace->TopLeft; corner <= _clockFace->TopRight; corner++)
  {
    _pixels.SetPixelColor(_clockFace->mapMinute(static_cast<ClockFace::Corners>(corner)), color);
  }
}

void Display::_circleAnimUpdate(const AnimationParam& param)
{
  if (param.state == AnimationState_Completed)
    {      
      switch(_circleCount) {
        case 0:
          DLOGLN("Testing green");
          _circleCenterX = 0;
          _circleCenterY = 5;
          _circleColor = HtmlColor(0x007f00);
          _bootAnimations.RestartAnimation(param.index);
          break;
        case 1:
          DLOGLN("Testing blue");
          _circleCenterX = 10;
          _circleCenterY = 5;
          _circleColor = HtmlColor(0x00007f);
          _bootAnimations.RestartAnimation(param.index);
          break;
        default:
          return;
      }
      _circleCount++;
  } else {
    float progress = NeoEase::QuarticOut(param.progress);
    int radius = progress * 15;
    _circle(_circleCenterX, _circleCenterY, radius, _circleColor);
    if (progress >= .95f && progress <= .98f) {
      _colorCornerPixels(_circleColor);
    }
  }
}

void Display::_fadeAll(uint8_t darkenBy)
{
    RgbColor color;
    for (uint16_t indexPixel = 0; indexPixel < _pixels.PixelCount(); indexPixel++)
    {
        color = _pixels.GetPixelColor(indexPixel);
        color.Darken(darkenBy);
        _pixels.SetPixelColor(indexPixel, color);
    }
}

void Display::_fadeAnimUpdate(const AnimationParam& param)
{
    if (param.state == AnimationState_Completed)
    {
      if (_bootAnimations.IsAnimating()) 
      {
        // Keep fading as long as the other animation is running.
        _fadeAll(5);
        _bootAnimations.RestartAnimation(param.index);
      }
      else 
      {
        DLOGLN("Boot animation complete");
        _update(200);
      }
    }
}

void Display::runBootAnimation()
{
    DLOGLN("Starting boot animation");
    DLOGLN("Testing red");
    _circleCenterX = 5;
    _circleCenterY = 10;
    _circleColor = HtmlColor(0x7f0000);
    _bootAnimations.StartAnimation(0, 20, [this](const AnimationParam& param) { _fadeAnimUpdate(param);});
    _bootAnimations.StartAnimation(1, 3000,[this](const AnimationParam& param) { _circleAnimUpdate(param);});
}