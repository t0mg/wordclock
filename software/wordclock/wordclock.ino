// Define debug to turn on debug logging.
// #define DEBUG 1
#include <Arduino.h>
#include <RTClib.h>

#include "src/logging.h"
#include "src/Display.h"
#include "src/ClockFace.h"
#include "src/Iot.h"

RTC_DS3231 rtc;
EnglishClockFace clockFace(ClockFace::LightSensorPosition::Bottom);
// FrenchClockFace clockFace(ClockFace::LightSensorPosition::Bottom);
Display display(clockFace);
Iot iot(&display, &rtc);

void setup()
{
  setupLogging();

  bool result = rtc.begin();
  DCHECK(result, "RTC didn't start");
  if (rtc.lostPower())
  {
    DCHECK("RTC lost power. Battery was removed ?");
  }

  display.setup();
  iot.setup();
}

void loop()
{
  DateTime now = rtc.now();
  display.updateForTime(now.hour(), now.minute(), now.second());
  display.loop();
  iot.loop();
}
