#include "logging.h"
#include "Iot.h"
#include <IotWebConfESP32HTTPUpdateServer.h>
#include "Timezones.h"
#include "clockfaces.h"
#include "Palette.h"

#include <NeoPixelBus.h>
#include <WiFi.h>
#include <uri/UriBraces.h>
#include <MQTT.h>
#include <esp_sntp.h>

// Name of this IoT object.
#define THING_NAME "WordClock"
// Initial WiFi access point password.
#define INITIAL_WIFI_AP_PASSWORD "password"
// IoT configuration version. Change this whenever IotWebConf object's
// configuration structure changes.
#define CONFIG_VERSION "6"
// Port used by the IotWebConf HTTP server.
#define WEB_SERVER_PORT 80
// Default timezone index from Timezones.h (Paris).
#define DEFAULT_TIMEZONE "385"
// Default language (EN)
#define DEFAULT_CLOCKFACE_LANGUAGE "0"
// HTTP OK status code.
#define HTTP_OK 200
// NTP server.
#define NTP_SERVER "pool.ntp.org"
// LDR MQTT topic publish interval in ms.
#define MQTT_LDR_PUBLISH_INTERVAL 15000

namespace
{
  // An SVG logo for the WordClock.
  extern const uint8_t logo_svg_start[] asm("_binary_src_logo_svg_start");
  extern const uint8_t logo_svg_end[] asm("_binary_src_logo_svg_end");

  // Matrix paint webpage.
  extern const uint8_t paint_html_start[] asm("_binary_src_paint_html_start");
  extern const uint8_t paint_html_end[] asm("_binary_src_paint_html_end");

  // Custom meta tags to be inserted in the head.
  const char CUSTOM_HTML_META[] = "<meta name=\"theme-color\" content=\"#121212\" />\n<link rel=\"icon\" href=\"logo.svg\" type=\"image/svg+xml\" />\n";

  // Custom Javascript block that will be added to the header.
  extern const uint8_t customscript_js_start[] asm("_binary_src_customscript_js_start");
  extern const uint8_t customscript_js_end[] asm("_binary_src_customscript_js_end");

  // Custom style added to the style tag.
  extern const uint8_t style_css_start[] asm("_binary_src_style_css_start");
  extern const uint8_t style_css_end[] asm("_binary_src_style_css_end");

  // Custom HTML element will be added at the beginning of the body element.
  const char CUSTOMHTML_BODY_INNER[] PROGMEM = "<header><div class=\"logoContainer\"><img class=\"logo\" src=\"logo.svg\"/></div></header>\n";

  class CustomHtmlFormatProvider : public iotwebconf::HtmlFormatProvider
  {
  protected:
    String getHead() override
    {
      String head = iotwebconf::HtmlFormatProvider::getHead();
      head.replace("{v}", THING_NAME);
      return head + String(FPSTR(CUSTOM_HTML_META));
    }
    String getScriptInner() override
    {
      return iotwebconf::HtmlFormatProvider::getScriptInner() +
            String((char *)customscript_js_start);
    }
    String getStyleInner() override
    {
      return iotwebconf::HtmlFormatProvider::getStyleInner() +
            String((char *)style_css_start);
    }
    String getBodyInner() override
    {
      return String(FPSTR(CUSTOMHTML_BODY_INNER)) +
            iotwebconf::HtmlFormatProvider::getBodyInner();
    }
  };
  // An instance must be created from the class defined above.
  CustomHtmlFormatProvider customHtmlFormatProvider;

  // Attempts to parse `str` as a color in "#RRGGBB" format. If it succeeds,
  // returns the parsed value. If it fails, returns `default_value`.
  RgbColor parseColorValue(const char *str, const RgbColor &default_value)
  {
    bool has_error = str[0] != '#' || str[7] != 0;
    for (int i = 1; i <= 6; i++)
    {
      has_error &= !isxdigit(str[i]);
    }
    if (has_error)
    {
      DLOG("[INFO] Could not parse color value \"");
      DLOG(str);
      DLOGLN("\".");
      return default_value;
    }

    const int parsed_value = strtol(str + 1, nullptr, 16);
    const uint8_t red = (parsed_value >> 16) & 0xFF;
    const uint8_t green = (parsed_value >> 8) & 0xFF;
    const uint8_t blue = parsed_value & 0xFF;
    return RgbColor(red, green, blue);
  }

  // Attempts to parse `str` as a number in the interval `[min_value, max_value].
  // If it succeeds, returns the parsed value. If it fails, returns
  // `default_value`.
  int parseNumberValue(const char *str, int min_value, int max_value,
                       int default_value)
  {
    char *end_ptr = nullptr;
    int parsed_value = strtol(str, &end_ptr, 10);
    if (*end_ptr != 0 || parsed_value < min_value || parsed_value > max_value)
    {
      DLOG("[INFO] Could not parse number value \"");
      DLOG(str);
      DLOGLN("\".");
      return default_value;
    }
    return parsed_value;
  }

  // Attempts to parse `str` as a number 0 or 1 and casts it as a boolean. If it
  // fails, returns false.
  bool parseBooleanValue(const char *str)
  {
    return static_cast<bool>(parseNumberValue(str, 0, 1, 0));
  }

  // Attempts to parse `str` as a time in hh:mm:ss format. If it succeeds, returns
  // true and populates `hour`, `minute` and `second` with the corresponding time
  // values. If it fails, returns false and leaves other parameters unchanged.
  bool parseTimeValue(const char *str, uint8_t *hour, uint8_t *minute,
                      uint8_t *second)
  {
    unsigned int parsed_hour;
    unsigned int parsed_minute;
    unsigned int parsed_second;

    int result = sscanf(str, "%u:%u:%u", &parsed_hour, &parsed_minute,
                        &parsed_second);
    if (result != 3 || parsed_hour > 23 || parsed_minute > 59 ||
        parsed_second > 59)
    {
      Serial.print("[INFO] Could not parse time value \"");
      Serial.print(str);
      Serial.println("\".");
      return false;
    }

    *hour = parsed_hour;
    *minute = parsed_minute;
    *second = parsed_second;
    return true;
  }

} // namespace

Iot::Iot(Display *display, RTC_DS3231 *rtc)
    : web_server_(WEB_SERVER_PORT), display_(display), rtc_(rtc),

      display_group_("display_group", "Display"),
      boot_animation_param_(
          "Startup animation", "boot_animation_enabled", boot_animation_enabled_value_,
          IOT_CONFIG_VALUE_LENGTH, "1", 0, 1, 1, "style='width: 40px;' data-labels='Off|On'"),
      clockface_language_param_(
          "Clock face language", "clockface_language", clockface_language_value_, IOT_CONFIG_VALUE_LENGTH,
          DEFAULT_CLOCKFACE_LANGUAGE, DEFAULT_CLOCKFACE_LANGUAGE, "data-options='English|Dutch|French|Italian'"),
      show_ampm_param_(
          "AM/PM indicator", "show_ampm", show_ampm_value_,
          IOT_CONFIG_VALUE_LENGTH, "0", 0, 1, 1, "style='width: 40px;' data-labels='Off|On' data-controlledby='clockface_language' data-showon='0'"),
      ldr_sensitivity_param_(
          "Light sensor sensitivity", "ldr_sensitivity", ldr_sensitivity_value_,
          IOT_CONFIG_VALUE_LENGTH, "5", 0, 10, 1, "data-labels='Off'"),
      color_param_("Color", "color", color_value_,
                   IOT_CONFIG_VALUE_LENGTH, "#FFFFFF", "#RRGGBB",
                   "data-type='color' pattern='#[0-9a-fA-F]{6}' "
                   "style='border-width: 1px; padding: 1px;'"),

      time_group_("time_group", "Time"),
      ntp_enabled_param_(
          "Use network time (requires WiFi)", "ntp_enabled", ntp_enabled_value_,
          IOT_CONFIG_VALUE_LENGTH, "0", 0, 1, 1, "style='width: 40px;' data-labels='Off|On'"),
      timezone_param_(
          "Time zone", "timezone", timezone_value_, IOT_CONFIG_VALUE_LENGTH,
          DEFAULT_TIMEZONE, DEFAULT_TIMEZONE, locationOptions),
      manual_time_param_("Time", "time", manual_time_value_, IOT_CONFIG_VALUE_LENGTH,
                         "hh:mm:ss", nullptr, "data-type='time' pattern='\\d{1,2}:\\d{1,2}:\\d{1,2}' step='1' data-controlledby='ntp_enabled' data-showon='0'"),

      api_group_("api_group", "API"),
      api_enabled_param_(
          "Enable the (unsecure) API and <a href='/paint'>paint tool</a>", "api_enabled", api_enabled_value_,
          IOT_CONFIG_VALUE_LENGTH, "0", 0, 1, 1, "style='width: 40px;' data-labels='Off|On'"),
 
      mqtt_group_("mqtt_group", "MQTT"),
      mqtt_enabled_param_(
          "Note: when enabled, config updates trigger a reboot", "mqtt_enabled", mqtt_enabled_value_,
          IOT_CONFIG_VALUE_LENGTH, "0", 0, 1, 1, "style='width: 40px;' data-labels='Off|On'"),
      mqtt_server_param_("MQTT server", "mqtt_server", mqtt_server_value_, IOT_CONFIG_VALUE_LENGTH, nullptr, nullptr, "data-controlledby='mqtt_enabled' data-showon='1'"),
      mqtt_user_param_("MQTT user", "mqtt_user", mqtt_user_value_, IOT_CONFIG_VALUE_LENGTH, nullptr, nullptr, "data-controlledby='mqtt_enabled' data-showon='1'"),
      mqtt_password_param_("MQTT password", "mqtt_password", mqtt_password_value_, IOT_CONFIG_VALUE_LENGTH, nullptr, nullptr, "data-controlledby='mqtt_enabled' data-showon='1'"),

      iot_web_conf_(THING_NAME, &dns_server_, &web_server_,
                    INITIAL_WIFI_AP_PASSWORD, CONFIG_VERSION)
{
  this->show_ampm_value_[0] = '\0';
  this->ldr_sensitivity_value_[0] = '\0';
  this->color_value_[0] = '\0';
  this->ntp_enabled_value_[0] = '\0';
  this->timezone_value_[0] = '\0';
  this->api_enabled_value_[0] = '\0';
  this->mqtt_enabled_value_[0] = '\0';
  this->mqtt_server_value_[0] = '\0';
  this->mqtt_user_value_[0] = '\0';
  this->mqtt_password_value_[0] = '\0';

}

Iot::~Iot() {}

void Iot::clearTransientParams_()
{
  manual_time_value_[0] = '\0';
}

// Note that IotWebConf does not currently work with parameters that require an
// HTML checkbox input to set in the configuration portal, so we have to use the
// workaround of representing booleans as 0 or 1 integers.
void Iot::updateClockFromParams_()
{
  switch (parseNumberValue(clockface_language_value_, 0, 10, 0))
  {
  case 1:
  {
    display_->setClockFace(&clockFaceNL);
    DLOGLN("Language set to Dutch");
    break;
  }
  case 2:
  {
    display_->setClockFace(&clockFaceFR);
    DLOGLN("Language set to French");
    break;
  }
  case 3:
  {
    display_->setClockFace(&clockFaceIT);
    DLOGLN("Language set to Italian");
    break;
  }
  default:
  {
    display_->setClockFace(&clockFaceEN);
    DLOGLN("Language set to English");
    break;
  }
  }

  display_->setColor(
      parseColorValue(color_value_, RgbColor(255, 255, 255)));
  display_->setShowAmPm(parseBooleanValue(show_ampm_value_));
  display_->setSensorSentivity(parseNumberValue(ldr_sensitivity_value_, 0, 10, 5));

  if (parseBooleanValue(ntp_enabled_value_))
  {
    maybeSetRTCfromNTP_();
  }
  else
  {
    if (sntp_enabled())
    {
      sntp_stop();
    }
    setRTCfromConfig_();
  }
}

void Iot::setup()
{
  DCHECK(!initialized_, "[WARN] Trying to setup Iot multiple times.");

  this->show_ampm_value_[0] = '\0';
  this->ldr_sensitivity_value_[0] = '\0';
  this->color_value_[0] = '\0';
  this->ntp_enabled_value_[0] = '\0';
  this->timezone_value_[0] = '\0';
  this->api_enabled_value_[0] = '\0';
  this->mqtt_enabled_value_[0] = '\0';
  this->mqtt_server_value_[0] = '\0';
  this->mqtt_user_value_[0] = '\0';
  this->mqtt_password_value_[0] = '\0';

  iot_web_conf_.setupUpdateServer(
      [this](const char *updatePath)
      { http_updater_.setup(&web_server_, updatePath); },
      [this](const char *userName, char *password)
      { http_updater_.updateCredentials(userName, password); });

  display_group_.addItem(&boot_animation_param_);
  display_group_.addItem(&clockface_language_param_);
  display_group_.addItem(&show_ampm_param_);
  display_group_.addItem(&ldr_sensitivity_param_);
  display_group_.addItem(&color_param_);
  iot_web_conf_.addParameterGroup(&display_group_);

  time_group_.addItem(&ntp_enabled_param_);
  time_group_.addItem(&timezone_param_);
  time_group_.addItem(&manual_time_param_);
  iot_web_conf_.addParameterGroup(&time_group_);

  api_group_.addItem(&api_enabled_param_);
  iot_web_conf_.addParameterGroup(&api_group_);

  mqtt_group_.addItem(&mqtt_enabled_param_);
  mqtt_group_.addItem(&mqtt_server_param_);
  mqtt_group_.addItem(&mqtt_user_param_);
  mqtt_group_.addItem(&mqtt_password_param_);
  iot_web_conf_.addParameterGroup(&mqtt_group_);

  iot_web_conf_.setConfigSavedCallback([this]()
                                       { handleConfigSaved_(); });

  iot_web_conf_.setFormValidator([this](iotwebconf::WebRequestWrapper *webRequestWrapper)
                                 { return formValidator_(webRequestWrapper); });

  iot_web_conf_.setWifiConnectionCallback([this]()
                                          { handleWifiConnection_(); });

  iot_web_conf_.setHtmlFormatProvider(&customHtmlFormatProvider);

  iot_web_conf_.getSystemParameterGroup()->label = "Network";
  iot_web_conf_.getThingNameParameter()->label = "Clock name";
  iot_web_conf_.getApPasswordParameter()->label = "AP password (login: admin)";

  iot_web_conf_.init();

  clearTransientParams_();
  updateClockFromParams_();

  web_server_.on("/", [this]  { handleHttpToRoot_(); });
  web_server_.onNotFound([this] { iot_web_conf_.handleNotFound(); });
  web_server_.on("/logo.svg", [this]() {
    web_server_.send(200, "image/svg+xml", (char *)logo_svg_start);
  });

  web_server_.on(UriBraces("/api/matrix/set/{}"), [this]() {
    if (parseBooleanValue(api_enabled_value_)) {    
      String payload = web_server_.pathArg(0);
      setMatrixFromPayload_(payload);
      web_server_.send(200, "text/plain", "OK");
    } else {
      web_server_.send(403, "text/plain", "API not enabled");
    }
  });
  web_server_.on("/api/matrix/unset", [this]() {
    if (parseBooleanValue(api_enabled_value_)) {    
      display_->clearMatrix();
      web_server_.send(200, "text/plain", "OK");
    } else {
      web_server_.send(403, "text/plain", "API not enabled");
    }
  });
  web_server_.on("/api/color/get", [this]() {
    if (parseBooleanValue(api_enabled_value_)) {
      char hexColor[10];
      HtmlColor(display_->getColor()).ToNumericalString(hexColor, 9);    
      web_server_.send(200, "text/plain", hexColor);
    } else {
      web_server_.send(403, "text/plain", "API not enabled");
    }
  });
  web_server_.on("/paint", [this]() {
    if (parseBooleanValue(api_enabled_value_)) {    
      web_server_.send(200, "text/html", (char *)paint_html_start);
    } else {
      web_server_.send(403, "text/plain", "API not enabled");
    }
  });
  web_server_.on("/wifilist", [this]() {
    long s = millis();

    int n = WiFi.scanComplete();
    Serial.print("Scan complete: ");
    Serial.println(n);
    if (n  == -2 ) {
      // not yet scanning.
      WiFi.scanNetworks(true);
      web_server_.send(200, "text/html", (char *)"");
      Serial.println("Scan started\n");
    } else if ( n == -1 ) {
      // still scanning.
      web_server_.send(200, "text/html", (char *)"");
    } else {
      if (n == 0) {
        web_server_.send(200, "text/html", (char *)"");
      } else {
        String str = "";
        for (int i = 0; i < n; ++i) {
          // Print SSID and RSSI for each network found
          str += WiFi.SSID(i) + "\n";
        }
        web_server_.send(200, "text/html", (char *)str.c_str());
      }
      WiFi.scanDelete();
    }
    long e = millis();
    Serial.println(e - s);
  });

  if (parseBooleanValue(mqtt_enabled_value_))
  {
    String prefix(iot_web_conf_.getThingName());
    prefix.toLowerCase();
    prefix.replace(" ", "");
    mqtt_topic_prefix_ = prefix;
    mqtt_client_.begin(mqtt_server_value_, net_);
    mqtt_client_.onMessage([this](String &topic, String &payload)
                           { mqttMessageReceived_(topic, payload); });
    String availabilityTopic = mqtt_topic_prefix_ + "/availability";
    mqtt_client_.setWill(availabilityTopic.c_str(), "offline");
  }

  if (parseBooleanValue(boot_animation_enabled_value_))
  {
    display_->runBootAnimation();
  }

  initialized_ = true;
}

void Iot::loop()
{
  DCHECK(initialized_, "[ERROR] Iot not initialized, loop aborted.");
  if (initialized_)
  {
    iot_web_conf_.doLoop();

    if (parseBooleanValue(mqtt_enabled_value_))
    {
      mqtt_client_.loop();
      if (needs_mqtt_connect_)
      {
        if (connectMQTT_())
        {
          needs_mqtt_connect_ = false;
        }
      }
      else if (iot_web_conf_.getState() == iotwebconf::OnLine)
      {
        if (mqtt_client_.connected())
        {
          unsigned long now = millis();
          if (MQTT_LDR_PUBLISH_INTERVAL < now - last_mqtt_report_)
          {
            last_mqtt_report_ = now;
            uint16_t sensorValue = display_->getRawSensorValue();
            DLOG("LDR value ");
            DLOGLN(sensorValue);
            mqtt_client_.publish(
                mqtt_topic_prefix_ + "/sensor/ldr",
                (String)sensorValue, true /* retained */, 0 /* QoS */);
          }
        }
        else
        {
          DLOGLN("MQTT reconnect");
          connectMQTT_();
        }
      }
    }
  }

  if (needs_reboot_)
  {
    DLOGLN("Rebooting after 1 second.");
    iot_web_conf_.delay(1000);
    ESP.restart();
  }
}

/**
 * Small hack, to get access to rtc from inside this
 * lowlevel callback from sntp.
 */
Iot *iot_sntp_global = nullptr;
void time_sync_notification_cb(struct timeval *tv)
{
  DLOGLN("Notification of a time synchronization event");
  if (tv->tv_sec > 0)
  {
    time_t now = tv->tv_sec;
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    iot_sntp_global->get_rtc()->adjust(
        DateTime(timeinfo.tm_year, timeinfo.tm_mon, timeinfo.tm_mday,
                 timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec));
    DLOG("Interval for sync: ");
    DLOGLN(sntp_get_sync_interval());
  }
}

void Iot::maybeSetRTCfromNTP_()
{
  if (!parseBooleanValue(ntp_enabled_value_))
  {
    DLOGLN("NTP time setting is disabled.");
    return;
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    DLOGLN("Wifi not connected, cannot set time from NTP.");
    return;
  }

  int tz = parseNumberValue(timezone_value_, 0, 459, 0);

  esp_netif_init();
  if (sntp_enabled())
  {
    sntp_stop();
  }
  sntp_setoperatingmode(SNTP_OPMODE_POLL);
#if SNTP_GET_SERVERS_FROM_DHCP || SNTP_GET_SERVERS_FROM_DHCPV6
  DLOGLN("Enable from dhcp");
  sntp_servermode_dhcp(1);
#endif
  sntp_setservername(0, (char *)NTP_SERVER);
  sntp_init();
  setenv("TZ", posix[tz], 1);
  tzset();

  iot_sntp_global = this;
  sntp_set_time_sync_notification_cb(time_sync_notification_cb);
}

void Iot::setRTCfromConfig_()
{
  const DateTime now = rtc_->now();
  uint16_t year = now.year();
  uint8_t month = now.month();
  uint8_t day = now.day();
  uint8_t hour = now.hour();
  uint8_t minute = now.minute();
  uint8_t second = now.second();
  bool datetime_changed = false;

  if (manual_time_value_[0] != 0 && parseTimeValue(manual_time_value_, &hour, &minute, &second))
  {
    datetime_changed = true;
  }

  if (datetime_changed)
  {
    rtc_->adjust(DateTime(year, month, day, hour, minute, second));
  }
}

void Iot::handleHttpToRoot_()
{
  // -- Let IotWebConf test and handle captive portal requests.
  if (iot_web_conf_.handleCaptivePortal())
  {
    // -- Captive portal request were already served.
    return;
  }

  clearTransientParams_();
  iot_web_conf_.handleConfig();
}

bool Iot::formValidator_(iotwebconf::WebRequestWrapper *webRequestWrapper)
{
  DLOGLN("Validating form.");
  bool valid = true;
  if (webRequestWrapper->arg(mqtt_enabled_param_.getId()).equals("1"))
  {
    int l = webRequestWrapper->arg(mqtt_server_param_.getId()).length();
    if (l < 3)
    {
      mqtt_server_param_.errorMessage = "Please provide at least 3 characters!";
      valid = false;
    }
  }
  return valid;
}

void Iot::handleConfigSaved_()
{
  DLOGLN("Configuration was updated.");
  if (parseBooleanValue(mqtt_enabled_value_))
  {
    needs_reboot_ = true;
  }
  else
  {
    updateClockFromParams_();
  }
}

void Iot::handleWifiConnection_()
{
  DLOGLN("Wifi connected.");
  if (parseBooleanValue(mqtt_enabled_value_))
  {
    needs_mqtt_connect_ = true;
  }
  maybeSetRTCfromNTP_();
}

bool Iot::connectMQTT_()
{
  unsigned long now = millis();
  if (1000 > now - last_mqtt_connection_attempt_)
  {
    // Do not repeat within 1 sec.
    return false;
  }
  DLOGLN("Connecting to MQTT server...");
  if (!connectMqttOptions_())
  {
    last_mqtt_connection_attempt_ = now;
    return false;
  }
  DLOGLN("Connected!");

  mqtt_client_.subscribe(mqtt_topic_prefix_ + "/light/color/set");
  mqtt_client_.subscribe(mqtt_topic_prefix_ + "/light/switch/set");
  mqtt_client_.subscribe(mqtt_topic_prefix_ + "/light/matrix/set");
  mqtt_client_.subscribe(mqtt_topic_prefix_ + "/light/matrix/unset");

  // Update availability.
  mqtt_client_.publish(mqtt_topic_prefix_ + "/availability", "online", true /* retained */, 0 /* QoS */);
  // Publish initial ON state for the switch.
  toggleDisplay_("ON");
  // Publish current color value.
  mqtt_client_.publish(
      mqtt_topic_prefix_ + "/light/color",
      rgbToMqttString_(parseColorValue(color_value_, RgbColor(0, 0, 0))), true /* retained */, 0 /* QoS */);

  return true;
}

bool Iot::connectMqttOptions_()
{
  bool result;
  if (mqtt_password_value_[0] != '\0')
  {
    result = mqtt_client_.connect(iot_web_conf_.getThingName(), mqtt_user_value_, mqtt_password_value_);
  }
  else if (mqtt_user_value_[0] != '\0')
  {
    result = mqtt_client_.connect(iot_web_conf_.getThingName(), mqtt_user_value_);
  }
  else
  {
    result = mqtt_client_.connect(iot_web_conf_.getThingName());
  }
  return result;
}

bool Iot::mqttStringToRgb_(String payload, RgbColor *color)
{
  int tokens[3];
  int tokenIndex = 0;
  for (int i = 0; i < 2; i++)
  {
    int nextComma = payload.indexOf(",");
    if (nextComma == -1)
    {
      return false;
    }
    String sub = payload.substring(0, nextComma);
    tokens[i] = sub.toInt();
    payload = payload.substring(nextComma + 1, payload.length());
  }
  if (payload.length() > 3)
  {
    return false;
  }
  tokens[2] = payload.toInt();
  *color = RgbColor(tokens[0], tokens[1], tokens[2]);
  return true;
}

void Iot::mqttMessageReceived_(String &topic, String &payload)
{
  DLOGLN("Incoming: " + topic + " - " + payload);
  if (topic == mqtt_topic_prefix_ + "/light/color/set")
  {
    RgbColor color;
    if (mqttStringToRgb_(payload, &color))
    {
      display_->setColor(color);
      mqtt_client_.publish(
          mqtt_topic_prefix_ + "/light/color",
          rgbToMqttString_(color), true /* retained */, 0 /* QoS */);
    }
  }
  else if (topic == mqtt_topic_prefix_ + "/light/switch/set")
  {
    toggleDisplay_(payload);
  }
  else if (topic == mqtt_topic_prefix_ + "/light/matrix/set")
  {
    setMatrixFromPayload_(payload);
  }
  else if (topic == mqtt_topic_prefix_ + "/light/matrix/unset")
  {
    display_->clearMatrix();
  }
}

void Iot::toggleDisplay_(String payload)
{
  if (payload == "ON")
  {
    display_->setOn();
    mqtt_client_.publish(
        mqtt_topic_prefix_ + "/light/switch",
        "ON", true /* retained */, 0 /* QoS */);
  }
  else if (payload == "OFF")
  {
    display_->setOff();
    mqtt_client_.publish(
        mqtt_topic_prefix_ + "/light/switch",
        "OFF", true /* retained */, 0 /* QoS */);
  }
}

void Iot::setMatrixFromPayload_(String &payload) {
  if (payload.length() >= 110) {
    display_->setMatrix(Palette::stringToRgb(payload, display_->getColor()));
  } else {
    DLOG("Matrix payload is too short :");
    DLOGLN(payload.length());
  }
}
