#ifndef _IOT_H_
#define _IOT_H_

#include "Display.h"
#include "Timer.h"

#include <IotWebConf.h>
#include <RTClib.h>

// Maximum length of a single IoT configuration value.
#define IOT_CONFIG_VALUE_LENGTH 16

class Iot
{
public:
  Iot(Display *display, RTC_DS3231 *rtc);
  ~Iot();

  Iot(const Iot &) = delete;
  Iot &operator=(const Iot &) = delete;

  Iot(Iot &&) = delete;
  Iot &operator=(Iot &&) = delete;

  void setup();
  void loop();

private:
  // Clears the values of transient parameters.
  void clearTransientParams_();
  // Updates word clock's state to match the current configuration values.
  void updateClockFromParams_();
  // Checks for NTP setting and if enabled, attempts to update the RTC.
  void maybeSetRTCfromNTP_();
  // Sets the RTC from manual input
  void setRTCfromConfig_();

  // Handles HTTP requests to web server's "/" path.
  void handleHttpToRoot_();
  // Handles configuration changes.
  void handleConfigSaved_();
  // Handles wifi connexion success.
  void handleWifiConnection_();

  // Whether IoT configuration was initialized.
  bool initialized_ = false;

  // Configuration portal's DNS server.
  DNSServer dns_server_;
  // Configuration portal's web server.
  WebServer web_server_;
  // Server for OTA firmware update.
  HTTPUpdateServer http_updater_;

  // Word clock display.
  Display *display_ = nullptr;

  // RTC.
  RTC_DS3231 *rtc_ = nullptr;

  // NTP poll timer;
  Timer ntp_poll_timer_;

  // Enables NTP time setting.
  IotWebConfParameter ntp_enabled_param_;
  // Value of the NTP setting option.
  char ntp_enabled_value_[IOT_CONFIG_VALUE_LENGTH];

  // The timezone index from available options.
  IotWebConfParameter timezone_param_;
  // Index of the selected timezone.
  char timezone_value_[IOT_CONFIG_VALUE_LENGTH];

  // Manual date setting. Transient.
  IotWebConfParameter manual_date_param_;
  // Date parameter value.
  char manual_date_value_[IOT_CONFIG_VALUE_LENGTH];

  // Manual time setting. Transient.
  IotWebConfParameter manual_time_param_;
  // Time parameter value.
  char manual_time_value_[IOT_CONFIG_VALUE_LENGTH];

  // Enable AM/PM display.
  IotWebConfParameter show_ampm_param_;
  // Value of the LDR sensitivity parameter.
  char show_ampm_value_[IOT_CONFIG_VALUE_LENGTH];

  // Sensitivity parameter for the LDR.
  IotWebConfParameter ldr_sensitivity_param_;
  // Value of the LDR sensitivity parameter.
  char ldr_sensitivity_value_[IOT_CONFIG_VALUE_LENGTH];

  // Text color parameter.
  IotWebConfParameter color_param_;
  // Value of the color parameter.
  char color_value_[IOT_CONFIG_VALUE_LENGTH];

  // Config form separators.
  IotWebConfSeparator time_separator_;
  IotWebConfSeparator display_separator_;

  // IotWebConf interface handle.
  IotWebConf iot_web_conf_;
};

#endif // _IOT_H_
