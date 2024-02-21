#pragma once

#include <vector>

class ClockFace
{
public:
  static int pixelCount();

  // The orientation of the clock is infered from where the light sensor is.
  enum class LightSensorPosition
  {
    Bottom,
    Top
  };

  ClockFace(LightSensorPosition position);

  // Rotates the display.
  void setLightSensorPosition(LightSensorPosition position);

  // Updates the state by setting to true all the LEDs that need tobe turned on
  // for the given time. Returns false if there is no change since last update,
  // in which case the state is not updated.
  virtual bool stateForTime(int hour, int minute, int second, bool show_ampm) = 0;

  // Returns the state of all LEDs as pixels. Updated when updateStateForTime()
  // is called.
  const std::vector<bool> &getState() { return _state; };

  // Returns the index of the LED in the strip given a position on the grid.
  uint16_t map(int16_t x, int16_t y);

  // The first four LED are the corner ones, counting minutes. They are assumed
  // to be wired in clockwise order, starting from the light sensor position.
  // mapMinute() returns the proper index based on desired location taking
  // orientation into account..
  enum Corners
  {
    TopLeft,
    BottomLeft,
    BottomRight,
    TopRight
  };
  uint16_t mapMinute(Corners corner);

  // Clear the display.
  void clearDisplay();

protected:
  // Lights up a segment in the state.
  void updateSegment(int x, int y, int length);

  // To avoid refreshing to often, this stores the time of the previous UI
  // update. If nothing changed, there will be no interuption of animations.
  int _hour, _minute, _second;
  bool _show_ampm;

  LightSensorPosition _position;

  // Stores the bits of the clock that need to be turned on.
  std::vector<bool> _state;
};

class FrenchClockFace : public ClockFace
{
public:
  FrenchClockFace(LightSensorPosition position) : ClockFace(position){};

  virtual bool stateForTime(int hour, int minute, int second, bool show_ampm);
};

class EnglishClockFace : public ClockFace
{
public:
  EnglishClockFace(LightSensorPosition position) : ClockFace(position){};

  virtual bool stateForTime(int hour, int minute, int second, bool show_ampm);
};

class DutchClockFace : public ClockFace
{
public:
  DutchClockFace(LightSensorPosition position) : ClockFace(position){};

  virtual bool stateForTime(int hour, int minute, int second, bool show_ampm);
};

class ItalianClockFace : public ClockFace
{
public:
  ItalianClockFace(LightSensorPosition position) : ClockFace(position){};

  virtual bool stateForTime(int hour, int minute, int second, bool show_ampm);
};
