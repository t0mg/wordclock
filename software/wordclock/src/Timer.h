#include <functional>
#include <Arduino.h>

/*
 * Basic ESP32 interrupt timer helper.
 */
class Timer
{
public:
  Timer();

  // Must be called from the ino setup and loop.
  void setup(std::function<void()> callback, uint32_t delay_seconds);
  void loop();
  void start();
  void stop();

private:
  std::function<void()> callback_ = NULL;
  uint32_t delay_seconds_ = 1;
  hw_timer_t *timer_ = NULL;
};
