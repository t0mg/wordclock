#include "logging.h"
#include "Iot.h"
#include <IotWebConfESP32HTTPUpdateServer.h>
#include "Timezones.h"
#include "clockfaces.h"

#include <NeoPixelBus.h>
#include <WiFi.h>
#include <MQTT.h>
#include <esp_sntp.h>

// Name of this IoT object.
#define THING_NAME "WordClock"
// Initial WiFi access point password.
#define INITIAL_WIFI_AP_PASSWORD "12345678"
// IoT configuration version. Change this whenever IotWebConf object's
// configuration structure changes.
#define CONFIG_VERSION "5"
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

namespace
{
  // An SVG logo for the Word Clock
  const char LOGO_DATA_URI[] PROGMEM = "data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 480 480'%3E%3Cpath d='M0 0h480v480H0z'/%3E%3Cg fill='%23fff'%3E%3Cpath d='M150.692 163.411h-6.563l-11.522-38.242q-.82-2.54-1.836-6.406-1.016-3.867-1.055-4.649-.86 5.157-2.735 11.29L115.81 163.41h-6.563l-15.195-57.109h7.031l9.024 35.273q1.875 7.422 2.734 13.438 1.055-7.148 3.125-13.984l10.234-34.727h7.031l10.742 35.04q1.875 6.054 3.164 13.671.742-5.547 2.813-13.516l8.984-35.195h7.031z' aria-label='W'/%3E%3Cpath d='M268.677 134.718q0 13.711-6.954 21.562-6.914 7.852-19.258 7.852-12.617 0-19.492-7.695-6.835-7.735-6.835-21.797 0-13.945 6.875-21.602 6.875-7.696 19.53-7.696 12.306 0 19.22 7.813t6.914 21.562zm-45.508 0q0 11.602 4.921 17.617 4.961 5.977 14.375 5.977 9.493 0 14.336-5.977t4.844-17.617q0-11.523-4.844-17.46-4.804-5.977-14.258-5.977-9.492 0-14.453 6.015-4.922 5.977-4.922 17.422z' aria-label='O'/%3E%3Cpath d='M154.135 244.514q0 14.14-7.696 21.64-7.656 7.462-22.07 7.462h-15.82v-57.11h17.5q13.32 0 20.703 7.383t7.383 20.625zm-7.032.234q0-11.172-5.625-16.836-5.586-5.664-16.64-5.664h-9.65v45.625h8.087q11.875 0 17.852-5.82 5.976-5.86 5.976-17.305z' aria-label='D'/%3E%3Cpath d='M339.34 139.658v23.75h-6.641v-57.109h15.664q10.508 0 15.508 4.023 5.039 4.024 5.039 12.11 0 11.327-11.484 15.311l15.508 25.664h-7.852l-13.828-23.75zm0-5.703h9.101q7.031 0 10.312-2.773 3.281-2.813 3.281-8.399 0-5.664-3.36-8.164-3.32-2.5-10.702-2.5h-8.633z' aria-label='R'/%3E%3Cpath d='M245.577 219.873q-9.414 0-14.883 6.289-5.43 6.25-5.43 17.148 0 11.211 5.235 17.344 5.273 6.094 15 6.094 5.976 0 13.633-2.149v5.82q-5.938 2.227-14.648 2.227-12.617 0-19.492-7.656-6.836-7.656-6.836-21.758 0-8.828 3.281-15.469 3.32-6.64 9.531-10.234 6.25-3.594 14.688-3.594 8.985 0 15.703 3.282l-2.812 5.703q-6.485-3.047-12.97-3.047z' aria-label='C'/%3E%3Cpath d='M156.44 354.9q0 13.711-6.953 21.562-6.914 7.852-19.258 7.852-12.617 0-19.492-7.696-6.836-7.734-6.836-21.797 0-13.945 6.875-21.602 6.875-7.695 19.531-7.695 12.305 0 19.219 7.813t6.914 21.562zm-45.508 0q0 11.602 4.922 17.617 4.96 5.977 14.375 5.977 9.492 0 14.336-5.977t4.844-17.617q0-11.523-4.844-17.461-4.805-5.977-14.258-5.977-9.492 0-14.453 6.016-4.922 5.977-4.922 17.422z' aria-label='O'/%3E%3Cpath d='M336.73 273.613v-57.109h6.641v51.094h25.195v6.016z' aria-label='L'/%3E%3Cpath d='M373.956 383.337h-7.813l-20.82-27.695-5.976 5.312v22.383h-6.64v-57.109h6.64v28.32l25.898-28.32h7.852l-22.97 24.805z' aria-label='K'/%3E%3Cpath d='M245.577 329.87q-9.414 0-14.883 6.29-5.43 6.25-5.43 17.147 0 11.211 5.235 17.344 5.273 6.094 15 6.094 5.976 0 13.633-2.148v5.82q-5.938 2.226-14.648 2.226-12.617 0-19.492-7.656-6.836-7.656-6.836-21.758 0-8.828 3.281-15.469 3.32-6.64 9.531-10.234 6.25-3.594 14.688-3.594 8.985 0 15.703 3.282l-2.812 5.703q-6.485-3.047-12.97-3.047z' aria-label='C'/%3E%3C/g%3E%3Cg fill='%23fff'%3E%3Ccircle cx='37.785' cy='37.968' r='7.906'/%3E%3Ccircle cx='439.98' cy='38.047' r='7.906'/%3E%3Ccircle cx='37.785' cy='440.74' r='7.906'/%3E%3Ccircle cx='439.98' cy='440.82' r='7.906'/%3E%3C/g%3E%3C/svg%3E";

  // Custom meta tags to be inserted in the head.
  const char CUSTOM_HTML_META_START[] = "<meta name=\"theme-color\" content=\"#121212\" />\n<link rel=\"icon\" href=\"";
  const char CUSTOM_HTML_META_END[] = "\" type=\"image/svg+xml\" />\n";

  // Custom Javascript block that will be added to the header.
  // See customconfig.js for human-readable version.
  const char CUSTOMHTML_SCRIPT_INNER[] PROGMEM = "document.addEventListener(\"DOMContentLoaded\",function(e){document.getElementById(\"iwcWifiSsid\")?.value&&(e=document.getElementById(\"iwcSys\"),e.parentElement.removeChild(e),document.querySelector(\"form\").insertBefore(e,document.querySelector(\"button[type=submit]\")));document.querySelectorAll(\"[data-type]\").forEach(function(a){a.type=a.getAttribute(\"data-type\")});document.querySelectorAll(\"input[type=password]\").forEach(function(a){var b=document.createElement(\"input\");b.classList.add(\"pwtoggle\");b.type=\
\"button\";b.value=\"\\ud83d\\udc41\\ufe0f\";a.insertAdjacentElement(\"afterend\",b);b.onclick=function(){\"password\"===a.type?(a.type=\"text\",b.value=\"\\ud83d\\udd12\"):(a.type=\"password\",b.value=\"\\ud83d\\udc41\\ufe0f\")}});(e=document.querySelector(\"form\"))&&e.addEventListener(\"submit\",function(){var a=document.querySelector(\"button[type=submit]\");a.innerText=\"Saving...\";a.toggleAttribute(\"disabled\",!0)});document.querySelectorAll(\"input[data-options]\").forEach(function(a){var b=a.value,f=a.getAttribute(\"data-options\").split(\"|\"),\
c=document.createElement(\"select\");c.name=a.name;c.id=a.id;\"\"===b&&c.appendChild(document.createElement(\"option\"));var d=null;f.forEach(function(l,m){var g=l.split(\"/\"),h=l;1<g.length?(h=g.splice(0,1)[0],d&&h==d.label||(d&&c.appendChild(d),d=document.createElement(\"optgroup\"),d.label=h),h=g.join(\" / \")):d&&(c.appendChild(d),d=null);g=document.createElement(\"option\");g.value=m;g.innerText=h;m==b&&g.toggleAttribute(\"selected\");d?d.appendChild(g):c.appendChild(g)});d&&c.appendChild(d);a.id+=\"-d\";f=a.getAttribute(\"data-controlledby\");\
var n=a.getAttribute(\"data-showon\");f&&n&&(c.setAttribute(\"data-controlledby\",f),c.setAttribute(\"data-showon\",n));a.insertAdjacentElement(\"beforebegin\",c);a.parentElement.removeChild(a)});document.querySelectorAll(\"input[type=range]\").forEach(function(a){var b=a.getAttribute(\"data-labels\"),f=b&&b.split(\"|\");b=function(){a.setAttribute(\"data-label\",f?f[parseInt(a.value,10)]||a.value:a.value)};a.oninput=b;b()});document.querySelectorAll(\"[data-controlledby]\").forEach(function(a){var b=document.getElementById(a.getAttribute(\"data-controlledby\")),\
f=a.getAttribute(\"data-showon\").split(\"|\"),c=function(){a.parentElement.style.display=0>f.indexOf(b.value+\"\")?\"none\":\"\"};b.addEventListener(\"change\",c);c()});var k=document.querySelector(\"input[type=color]\");k&&(e=function(){document.querySelector(\".logoContainer\").style.backgroundColor=k.value},k.addEventListener(\"input\",e),e());document.body.classList.add(\"ready\")});";

  // Custom style added to the style tag.
  const char CUSTOMHTML_STYLE_INNER[] PROGMEM = "\n\
body {\
  color: #eee;\
  margin: 0;\
background: #121212;\
  font: 100% system-ui;}\
a {\
  color: #16a1e7;}\
div {\
  padding: 0;}\
.de {\
  background-color: transparent;}\
.de input {\
  background-color: #ffd4a6; }\
.em {\
  color: #ffb86d; }\
body > div > form, body > div > div {\
  display: none !important; }\
body > div:has(> form)::after {\
  content: 'LOADING...';\
  display: block;\
  font-size: 1.5em;\
  text-align: center;\
  margin-top: 50%;}\
body.ready > div > form, body.ready > div > div {\
  display: block !important;}\
body.ready > div:has(> form)::after {\
  display: none;}\
.logo {\
  width: 80vw;\
  max-width: 160px;\
  display: block;\
  mix-blend-mode: multiply;}\
.logoContainer {\
  background: white;\
  transition: background 3s;\
  display: inline-block;\
  margin: 40px auto;\
  padding: 0;}\
.logoContainer, fieldset {\
  box-shadow: 0px 0px 15px 1px #ffffff7d;}\
.pwtoggle {\
  padding: 0 5px;\
  position: absolute;\
  right: 3px;\
  top: 40px;\
  border: none;\
  background: transparent;\
  height: 34px;\
  outline: none;}\
fieldset {\
  padding: 40px 15px 20px;\
  margin-bottom: 40px;\
  border: none;\
  background: black;\
  border-radius: 0;\
  position: relative;\
  width: calc(100vw - 100px);\
  max-width: 440px;}\
fieldset > div {\
  position: relative;\
  margin: 0;\
  padding: 0;\
  display: flex;\
  flex-direction: column;}\
input, select {\
  margin: 0;\
  padding: 5px;\
  width: auto;\
  line-height: 20px;\
  border: none;}\
input[type=\"range\"] {\
  margin-left: 30px;\
  position: relative;\
  padding: 0;}\
input[type=\"range\"]:before {\
  content: attr(data-label);\
  color: #eee;\
  position: absolute;\
  left: -30px;\
  display: inline;\
  width: 25px;\
  line-height: 20px;}\
label {margin: 15px 0 5px;}\
legend, fieldset:first-child:after {\
  font-weight: lighter;\
  padding: 0;\
  font-size: 1.2em;\
  text-align: center;\
  position: absolute;\
  top: 15px;\
  display: block;\
  left: 0;\
  right: 0;\
  text-transform: uppercase;}\
fieldset:first-child:after {content: \"Network\";}\
button {\
  border-radius: 0;\
  text-transform: uppercase;}\
form + div {padding: 20px 0 15px 0;}\
body > div > div:last-child {\
  margin-top: -20px;\
  float: right;}\
\n";

  // Custom HTML element will be added at the beginning of the body element.
  const char CUSTOMHTML_BODY_INNER_START[] PROGMEM = "<header><div class=\"logoContainer\"><img class=\"logo\" src=\"";
  const char CUSTOMHTML_BODY_INNER_END[] PROGMEM = "\"/></div></header>\n";

  class CustomHtmlFormatProvider : public iotwebconf::HtmlFormatProvider
  {
  protected:
    String getHead() override
    {
      String head = iotwebconf::HtmlFormatProvider::getHead();
      head.replace("{v}", THING_NAME);
      return head + String(FPSTR(CUSTOM_HTML_META_START)) +
             String(FPSTR(LOGO_DATA_URI)) +
             String(FPSTR(CUSTOM_HTML_META_END));
    }
    String getScriptInner() override
    {
      return iotwebconf::HtmlFormatProvider::getScriptInner() +
             String(FPSTR(CUSTOMHTML_SCRIPT_INNER));
    }
    String getStyleInner() override
    {
      return iotwebconf::HtmlFormatProvider::getStyleInner() +
             String(FPSTR(CUSTOMHTML_STYLE_INNER));
    }
    String getBodyInner() override
    {
      return String(FPSTR(CUSTOMHTML_BODY_INNER_START)) +
             String(FPSTR(LOGO_DATA_URI)) +
             String(FPSTR(CUSTOMHTML_BODY_INNER_END)) +
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

      mqtt_group_("mqtt_group", "MQTT (work in progress)"),
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
    if (sntp_enabled()) {
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
  if (parseBooleanValue(boot_animation_enabled_value_))
  {
    display_->runBootAnimation();
  }
  web_server_.on("/", [this]
                 { handleHttpToRoot_(); });
  web_server_.onNotFound([this]
                         { iot_web_conf_.handleNotFound(); });

  if (parseBooleanValue(mqtt_enabled_value_))
  {
    mqtt_client_.begin(mqtt_server_value_, net_);
    mqtt_client_.onMessage([this](String &topic, String &payload)
                           { mqttMessageReceived_(topic, payload); });
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
      if (needs_MQTT_connect_)
      {
        if (connectMQTT_())
        {
          needs_MQTT_connect_ = false;
        }
      }
      else if ((iot_web_conf_.getState() == iotwebconf::OnLine) && (!mqtt_client_.connected()))
      {
        DLOGLN("MQTT reconnect");
        connectMQTT_();
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
    if ( tv->tv_sec > 0 ) {
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
  if (sntp_enabled()) {
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
    needs_MQTT_connect_ = true;
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

  mqtt_client_.subscribe("color/set");
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

void Iot::mqttMessageReceived_(String &topic, String &payload)
{
  DLOGLN("Incoming: " + topic + " - " + payload);
  // TODO Update config and trigger saveconfig.
  display_->setColor(
    parseColorValue(payload.c_str(), parseColorValue(color_value_, RgbColor(255, 255, 255))));
  // Doing this currently reboots the clock, which isn't great.
  // strncpy(
    // color_value_, payload.c_str(),
    // IOT_CONFIG_VALUE_LENGTH);
  // iot_web_conf_.saveConfig();
}