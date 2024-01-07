#include <Arduino.h>
#include <RTClib.h>
#include "src/logging.h"
#include "src/Display.h"
#include "src/ClockFace.h"
#include "src/Iot.h"
#include "src/nodo.h" // Nodo stuff
RTC_DS3231 rtc;
#ifdef NODO
EnglishClockFace clockFace(ClockFace::LightSensorPosition::Top);
#else
EnglishClockFace clockFace(ClockFace::LightSensorPosition::Bottom);
#endif
// FrenchClockFace clockFace(ClockFace::LightSensorPosition::Top);
// DutchClockFace clockFace(ClockFace::LightSensorPosition::Top);
Display display(clockFace);

Iot iot(&display, &rtc);

void setup()
{
  setupLogging();
#ifdef LED
  pinMode(LED, OUTPUT);
#endif
  Wire.begin(SDA, SCL);
  bool result = rtc.begin(); // pins for RTC swapped over on V1.0
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
