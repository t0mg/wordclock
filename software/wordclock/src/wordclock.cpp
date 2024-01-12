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
  bool result = rtc.begin(); // pins for RTC swapped over on V1.0
  DCHECK(result, "RTC didn't start");
  if (rtc.lostPower())
  {
    DCHECK("RTC lost power. Battery was removed ?");
  }

  display.setup();
  iot.setup();
}

long last_rtc_update = 0;
void loop()
{
  long ts_now = millis();
  if ((ts_now - last_rtc_update) > 99) {
    DateTime now = rtc.now();
    display.updateForTime(now.hour(), now.minute(), now.second());
    last_rtc_update = ts_now;
  }

  display.loop();
  iot.loop();
}