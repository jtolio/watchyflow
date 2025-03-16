#include "WatchyFace.h"
#include "Layout.h"

#define DARKMODE false

const uint8_t BATTERY_SEGMENT_WIDTH = 7;
const uint8_t BATTERY_SEGMENT_HEIGHT = 11;
const uint8_t BATTERY_SEGMENT_SPACING = 9;
const uint8_t WEATHER_ICON_WIDTH = 48;
const uint8_t WEATHER_ICON_HEIGHT = 32;


class LayoutBattery : public LayoutElement {
public:
  explicit LayoutBattery(float vbat) : vbat_(vbat) {}

  void size(uint16_t availableWidth, uint16_t availableHeight,
            uint16_t *width, uint16_t *height) override {
    *width = 37;
    *height = 21;
  }

  void draw(int16_t x0, int16_t y0,
            uint16_t availableWidth, uint16_t availableHeight,
            uint16_t *width, uint16_t *height) override {
    *width = 37;
    *height = 21;

    Watchy::Watchy::display.drawBitmap(x0, y0, battery, 37, 21,
                                       DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
    Watchy::Watchy::display.fillRect(x0 + 5, y0 + 5, 27, BATTERY_SEGMENT_HEIGHT,
                                     DARKMODE ? GxEPD_BLACK : GxEPD_WHITE);
    int8_t batteryLevel = 0;
    float VBAT = vbat_;
    if(VBAT > 4.0){
        batteryLevel = 3;
    }
    else if(VBAT > 3.6 && VBAT <= 4.0){
        batteryLevel = 2;
    }
    else if(VBAT > 3.20 && VBAT <= 3.6){
        batteryLevel = 1;
    }
    else if(VBAT <= 3.20){
        batteryLevel = 0;
    }

    for(int8_t batterySegments = 0; batterySegments < batteryLevel; batterySegments++){
        Watchy::Watchy::display.fillRect(
          x0 + 5 + (batterySegments * BATTERY_SEGMENT_SPACING),
          y0 + 5,
          BATTERY_SEGMENT_WIDTH, BATTERY_SEGMENT_HEIGHT,
          DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
    }
  }
private:
  float vbat_;
};

void WatchyFace::drawWatchFace() {
  display.fillScreen(DARKMODE ? GxEPD_BLACK : GxEPD_WHITE);

  uint16_t color = DARKMODE ? GxEPD_WHITE : GxEPD_BLACK;

  String timeStr = "";
  int displayHour = ((currentTime.Hour+11)%12)+1;
  if (displayHour < 10){
    timeStr += "0";
  }
  timeStr += String(displayHour) + ":";
  if (currentTime.Minute < 10){
    timeStr += "0";
  }
  timeStr += String(currentTime.Minute);

  weatherData currentWeather = getWeatherData();
  String weatherStr = String(currentWeather.weatherTemperature) +
      (currentWeather.isMetric ? "C" : "F");

  String dayOfWeekStr = dayShortStr(currentTime.Wday);
  String monthStr = monthShortStr(currentTime.Month);
  String dayOfMonthStr = String(currentTime.Day);
  if (currentTime.Day < 10) {
    dayOfMonthStr = String("0") + dayOfMonthStr;
  }

  LayoutFill elFill;
  LayoutPad elPad(5);

  LayoutText elTime(timeStr, &DSEG7_Classic_Bold_25, color);

  LayoutBitmap elWifi(WIFI_CONFIGURED ? wifi : wifioff, 26, 18, color);
  LayoutText elTemp(weatherStr, &Seven_Segment10pt7b, color);

  LayoutElement *elTempOrWifi = &elWifi;
  if (currentWeather.weatherConditionCode >= 0) {
    elTempOrWifi = &elTemp;
  }

  LayoutElement *elTempOrWifiCenteredElems[] = {&elFill, elTempOrWifi, &elFill};
  bool elTempOrWifiCenteredStretch[] = {true, false, true};
  LayoutRows elTempOrWifiCentered(3, elTempOrWifiCenteredElems, elTempOrWifiCenteredStretch);

  LayoutBattery elBattery(getBatteryVoltage());
  LayoutElement *elBatteryCenteredElems[] = {&elFill, &elBattery, &elFill};
  bool elBatteryCenteredStretch[] = {true, false, true};
  LayoutRows elBatteryCentered(3, elBatteryCenteredElems, elBatteryCenteredStretch);

  LayoutElement *elTopElems[] = {&elTime, &elFill, &elTempOrWifiCentered, &elPad, &elBatteryCentered};
  bool elTopStretch[] = {false, true, false, false, false};
  LayoutColumns elTop(5, elTopElems, elTopStretch);

  LayoutText elDateWords(dayOfWeekStr + ", " + monthStr + " " + dayOfMonthStr, &Seven_Segment10pt7b, color);
  LayoutRotate elDateRotated(&elDateWords, 3);
  LayoutElement *elDateElems[] = {&elDateRotated, &elFill};
  bool elDateStretch[] = {false, true};
  LayoutRows elDate(2, elDateElems, elDateStretch);

  LayoutFill elCalendar;

  LayoutElement *elMainElems[] = {&elDate, &elCalendar};
  bool elMainStretch[] = {false, true};
  LayoutColumns elMain(2, elMainElems, elMainStretch);

  LayoutElement *elScreenElems[] = {&elTop, &elPad, &elMain};
  bool elScreenStretch[] = {false, false, true};
  LayoutRows elScreen(3, elScreenElems, elScreenStretch);

  LayoutPadChild elPaddedScreen(&elScreen, 5, 5, 5, 5);

  uint16_t w, h;
  elPaddedScreen.draw(0, 0, 200, 200, &w, &h);
}
