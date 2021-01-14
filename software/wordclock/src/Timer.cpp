#include <Arduino.h>
#include "Timer.h"
#include "logging.h"

namespace
{

    portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
    volatile int interruptCounter = 0;

    void IRAM_ATTR onTimer_()
    {
        portENTER_CRITICAL_ISR(&timerMux);
        interruptCounter++;
        portEXIT_CRITICAL_ISR(&timerMux);
    }

} // namespace

Timer::Timer() {}

void Timer::setup(std::function<void()> callback, uint32_t delay_seconds)
{
    callback_ = callback;
    delay_seconds_ = delay_seconds;
}

void Timer::start()
{
    timer_ = timerBegin(3, 8000, 1); //div 80
    timerAttachInterrupt(timer_, &onTimer_, 1);
    timerAlarmWrite(timer_, delay_seconds_ * 10000, true); //1000ms
    timerAlarmEnable(timer_);
}

// start/stop the timer
void Timer::stop()
{
    if (timer_ == NULL)
    {
        return;
    }
    timerEnd(timer_);
    timer_ = NULL;
}

void Timer::loop()
{
    if (interruptCounter > 0)
    {
        portENTER_CRITICAL(&timerMux);
        interruptCounter--;
        portEXIT_CRITICAL(&timerMux);

        if (callback_ != NULL)
        {
            callback_();
        }
    }
}