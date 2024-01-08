#include <IotWebConf.h>

#define RANGE_ATTR_LENGTH 128

const char IOTWEBCONF_HTML_FORM_RANGE_PARAM[] PROGMEM =
  "<div class='{s}'><label for='{i}'>{b}</label>"
  "<input type='{t}' id='{i}' "
  "name='{i}' maxlength={l} placeholder='{p}' value='{v}' {c}/>"
  "<div class='em'>{e}</div></div>\n";

class IotRangeValueParameter : public iotwebconf::NumberParameter
{
public:
  IotRangeValueParameter(
    const char* label, const char* id, char* valueBuffer, int length,
    const char* defaultValue,
    int min, int max, int step, const char* extraAttributes) : iotwebconf::NumberParameter(
      label, id, valueBuffer, length, defaultValue)
    {
      snprintf(
        this->_rangeAttr, RANGE_ATTR_LENGTH,
        "min='%d' max='%d' step='%d' %s",
        min, max, step, extraAttributes);

      this->customHtml = this->_rangeAttr;
    };
protected:
  // Overrides
  virtual String renderHtml(
    bool dataArrived, bool hasValueFromPost, String valueFromPost) override
  {
    return TextParameter::renderHtml("range", hasValueFromPost, valueFromPost);
  };
  virtual String getHtmlTemplate()
  {
    return FPSTR(IOTWEBCONF_HTML_FORM_RANGE_PARAM);
  };

private:
  char _rangeAttr[RANGE_ATTR_LENGTH];
};
