//  Copyright 2023 Tom Granger

//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at

//      http://www.apache.org/licenses/LICENSE-2.0

//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
   
#include <Arduino.h>
#include <RTClib.h>
#include "src/logging.h"
#include "src/Display.h"
#include "src/ClockFace.h"
#include "src/Iot.h"
#include "src/nodo.h" // Nodo stuff
RTC_DS3231 rtc;
boolean rtc_found = false;

// TODO: refactor so we don't need to initialize another clockface here.
#ifdef NODO
auto pos = ClockFace::LightSensorPosition::Top;
#else
auto pos = ClockFace::LightSensorPosition::Bottom;
#endif

EnglishClockFace clockFace(pos);
Display display(&clockFace);

Iot iot(&display, &rtc);

void setup()
{
  setupLogging();
#ifdef LED_PIN
  pinMode(LED_PIN, OUTPUT);
#endif
#ifdef SDA_PIN
  Wire.begin(SDA_PIN, SCL_PIN);
#endif
  rtc_found = rtc.begin(); // pins for RTC swapped over on V1.0
  if (rtc_found) {
    DLOG("RTC detected");
    if (rtc.lostPower()) {
      DCHECK("RTC lost power. Battery was removed ?");
    }
  } else {
    DLOG("RTC not found");
  }
  iot.set_rtc_found(rtc_found);

  display.setup();
  iot.setup();
}

long last_rtc_update = 0;
void loop()
{
  long ts_now = millis();
  if ((ts_now - last_rtc_update) > 99) {
    if ( rtc_found ) {
      DateTime now = rtc.now();
      display.updateForTime(now.hour(), now.minute(), now.second());
    } else {
      struct tm timeinfo;
      // Get the local time from the esp's rtc.
      if (getLocalTime(&timeinfo)) {
        auto now = DateTime(timeinfo.tm_year, timeinfo.tm_mon, timeinfo.tm_mday,
                            timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        display.updateForTime(now.hour(), now.minute(), now.second());
      } else {
        display.updateForTime(0, 0, 0);
      }
    }
    last_rtc_update = ts_now;
  }

  display.loop();
  iot.loop();
}