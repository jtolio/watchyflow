#include "WatchyFace.h"
#include "Layout.h"
#include "Battery.h"
#include "Calendar.h"
#include <Fonts/FreeMono9pt7b.h>
#include "Seven_Segment10pt7b.h"
#include "DSEG7_Classic_Bold_25.h"
#include "icons.h"

#define DARKMODE false

const uint8_t MAX_CALENDAR_COLUMNS = 8;

const uint8_t WEATHER_ICON_WIDTH  = 48;
const uint8_t WEATHER_ICON_HEIGHT = 32;

RTC_DATA_ATTR dayEventsData calendarDay;
RTC_DATA_ATTR eventsData calendar[MAX_CALENDAR_COLUMNS];
RTC_DATA_ATTR alarmsData alarms;
RTC_DATA_ATTR uint8_t activeCalendarColumns;
RTC_DATA_ATTR time_t lastCalendarFetch;
RTC_DATA_ATTR char calendarError[32];
RTC_DATA_ATTR weatherData currentWeather;
RTC_DATA_ATTR time_t lastWeatherFetch;

void zeroError() {
  for (int i = 0; i < (sizeof(calendarError) / sizeof(calendarError[0])); i++) {
    calendarError[i] = 0;
  }
}

void WatchyFace::deviceReset() {
  activeCalendarColumns = 1;
  reset(&calendar[0]);
  reset(&alarms);
  reset(&calendarDay);
  lastCalendarFetch                   = 0;
  lastWeatherFetch                    = 0;
  currentWeather.weatherConditionCode = -1;
  zeroError();
}

void WatchyFace::postDraw() {
  time_t currentUnixTime = unixEpochTime(currentTime);
  if (currentUnixTime - 3600 <= lastCalendarFetch &&
      currentUnixTime - 3600 <= lastWeatherFetch) {
    return;
  }

  if (!connectWiFi()) {
    return;
  }

  if (currentUnixTime - 3600 > lastCalendarFetch) {
    fetchCalendar();
  }
  if (currentUnixTime - 3600 > lastWeatherFetch) {
    fetchWeather();
  }

  // turn off radios
  WiFi.mode(WIFI_OFF);
  btStop();
}

void WatchyFace::fetchCalendar() {
  HTTPClient http;
  http.setConnectTimeout(1000 * 10);
  http.setTimeout(1000 * 10);
  String calQueryURL =
      "http://mini2011.lan:8082/v0/account/{calendarAccountKey}";
  calQueryURL.replace("{calendarAccountKey}", settings.calendarAccountKey);
  http.begin(calQueryURL.c_str());
  int httpResponseCode = http.GET();
  if (httpResponseCode == 200) {
    zeroError();
    parseCalendar(http.getString());
  } else {
    String error(httpResponseCode);
    error.toCharArray(calendarError,
                      sizeof(calendarError) / sizeof(calendarError[0]));
  }
  http.end();
}

void WatchyFace::parseCalendar(String payload) {
  JSONVar parsed = JSON.parse(payload);
  if (!parsed.hasOwnProperty("status")) {
    return;
  }
  if (!parsed.hasOwnProperty("columns")) {
    return;
  }
  if (!parsed.hasOwnProperty("events")) {
    return;
  }
  String status = parsed["status"];
  if (status != "ok") {
    return;
  }

  lastCalendarFetch     = unixEpochTime(currentTime);
  activeCalendarColumns = (uint8_t)(int)parsed["columns"];
  if (activeCalendarColumns >= MAX_CALENDAR_COLUMNS) {
    activeCalendarColumns = MAX_CALENDAR_COLUMNS;
  }
  if (activeCalendarColumns <= 0) {
    activeCalendarColumns = 1;
  }
  for (int i = 0; i < activeCalendarColumns; i++) {
    reset(&calendar[i]);
  }
  reset(&calendarDay);
  reset(&alarms);

  JSONVar events = parsed["events"];
  for (int i = 0; i < events.length(); i++) {
    JSONVar event = events[i];
    if (!event.hasOwnProperty("summary")) {
      continue;
    }
    if (!event.hasOwnProperty("day")) {
      continue;
    }

    String summary = event["summary"];
    bool allDay    = (bool)event["day"];
    if (allDay) {
      if (!event.hasOwnProperty("start")) {
        continue;
      }
      if (!event.hasOwnProperty("end")) {
        continue;
      }
      String start = event["start"];
      String end   = event["end"];
      addEvent(&calendarDay, summary, start, end);
      continue;
    }

    if (!event.hasOwnProperty("start-unix")) {
      continue;
    }
    time_t startUnix = (time_t)(long)event["start-unix"];

    if (summary.indexOf(String("[WATCHY ALARM]")) >= 0) {
      summary.replace(String("[WATCHY ALARM]"), String(""));
      summary.trim();
      addAlarm(&alarms, summary, startUnix);
      continue;
    }

    if (!event.hasOwnProperty("column")) {
      continue;
    }
    int column = (int)event["column"];
    if (column >= activeCalendarColumns || column < 0) {
      continue;
    }

    if (!event.hasOwnProperty("end-unix")) {
      continue;
    }
    time_t endUnix = (time_t)(long)event["end-unix"];
    addEvent(&calendar[column], summary, startUnix, endUnix);
  }
}

void WatchyFace::fetchWeather() {
  currentWeather.isMetric = settings.weatherUnit == String("metric");

  HTTPClient http;
  http.setConnectTimeout(3000);
  String weatherQueryURL = settings.weatherURL;
  if (settings.cityID != "") {
    weatherQueryURL.replace("{cityID}", settings.cityID);
  } else {
    weatherQueryURL.replace("{lat}", settings.lat);
    weatherQueryURL.replace("{lon}", settings.lon);
  }
  weatherQueryURL.replace("{units}", settings.weatherUnit);
  weatherQueryURL.replace("{lang}", settings.weatherLang);
  weatherQueryURL.replace("{apiKey}", settings.weatherAPIKey);
  http.begin(weatherQueryURL.c_str());
  int httpResponseCode = http.GET();
  if (httpResponseCode == 200) {
    String payload                    = http.getString();
    JSONVar responseObject            = JSON.parse(payload);
    currentWeather.weatherTemperature = int(responseObject["main"]["temp"]);
    currentWeather.weatherConditionCode =
        int(responseObject["weather"][0]["id"]);
    gmtOffset = int(responseObject["timezone"]);
    syncNTP(gmtOffset);
    lastWeatherFetch = unixEpochTime(currentTime);
  }
  http.end();
}

void WatchyFace::drawWatchFace() {
  display.fillScreen(DARKMODE ? GxEPD_BLACK : GxEPD_WHITE);
  display.setTextWrap(false);

  uint16_t color = DARKMODE ? GxEPD_WHITE : GxEPD_BLACK;

  String timeStr  = "";
  int displayHour = ((currentTime.Hour + 11) % 12) + 1;
  if (displayHour < 10) {
    timeStr += "0";
  }
  timeStr += String(displayHour) + ":";
  if (currentTime.Minute < 10) {
    timeStr += "0";
  }
  timeStr += String(currentTime.Minute);

  String weatherStr = String(currentWeather.weatherTemperature) +
                      (currentWeather.isMetric ? "C" : "F");

  String dayOfWeekStr  = dayShortStr(currentTime.Wday);
  String monthStr      = monthShortStr(currentTime.Month);
  String dayOfMonthStr = String(currentTime.Day);
  if (currentTime.Day < 10) {
    dayOfMonthStr = String("0") + dayOfMonthStr;
  }

  LayoutFill elFill;
  LayoutSpacer elSpacer(5);

  LayoutText elTime(timeStr, &DSEG7_Classic_Bold_25, color);
  LayoutCenter elTimeCentered(&elTime);

  LayoutBitmap elWifi(WIFI_CONFIGURED ? wifi : wifioff, 26, 18, color);
  LayoutText elTemp(weatherStr, &Seven_Segment10pt7b, color);
  LayoutElement *elTempOrWifi = &elWifi;
  if (currentWeather.weatherConditionCode >= 0) {
    elTempOrWifi = &elTemp;
  }
  LayoutCenter elTempOrWifiCentered(elTempOrWifi);

  LayoutBattery elBattery(getBatteryVoltage(), color);
  LayoutCenter elBatteryCentered(&elBattery);

  LayoutElement *elTopElems[] = {&elTimeCentered, &elFill,
                                 &elTempOrWifiCentered, &elSpacer,
                                 &elBatteryCentered};
  bool elTopStretch[]         = {false, true, false, false, false};
  LayoutColumns elTop(5, elTopElems, elTopStretch);

  LayoutText elError(String(calendarError), &FreeMono9pt7b, color);
  LayoutCenter elErrorCenter(&elError);
  LayoutRotate elErrorRotated(&elErrorCenter, 3);

  LayoutText elDateWords(dayOfWeekStr + ", " + monthStr + " " + dayOfMonthStr,
                         &Seven_Segment10pt7b, color);
  LayoutRotate elDateRotated(&elDateWords, 3);
  LayoutElement *elDateElems[] = {&elDateRotated, &elFill, &elErrorRotated};
  bool elDateStretch[]         = {false, true, false};
  LayoutRows elDate(3, elDateElems, elDateStretch);

  CalendarDayEvents elCalendarDay(&calendarDay, currentTime, color);

  CalendarHourBar elCalHourBar(currentTime, color);

  CalendarColumn elCals[activeCalendarColumns];
  LayoutElement *elCalColElems[activeCalendarColumns + 1];
  bool elCalColStretch[activeCalendarColumns + 1];
  elCalColElems[0]   = &elCalHourBar;
  elCalColStretch[0] = false;
  for (int i = 0; i < activeCalendarColumns; i++) {
    elCals[i]              = CalendarColumn(&calendar[i], this, color);
    elCalColElems[i + 1]   = &elCals[i];
    elCalColStretch[i + 1] = true;
  }
  LayoutColumns elCalColumns(activeCalendarColumns + 1, elCalColElems,
                             elCalColStretch);

  LayoutElement *elCalendarPartsElems[] = {&elCalendarDay, &elCalColumns};
  bool elCalendarPartsStretch[]         = {false, true};
  LayoutRows elCalendarParts(2, elCalendarPartsElems, elCalendarPartsStretch);

  LayoutBorder elCalendarBorder(&elCalendarParts, true, false, true, true,
                                color);

  LayoutElement *elMainElems[] = {&elDate, &elSpacer, &elCalendarBorder};
  bool elMainStretch[]         = {false, false, true};
  LayoutColumns elMain(3, elMainElems, elMainStretch);

  CalendarAlarms elAlarms(&alarms, this, color);

  LayoutElement *elScreenElems[] = {&elTop, &elSpacer, &elMain, &elAlarms};
  bool elScreenStretch[]         = {false, false, true, false};
  LayoutRows elScreen(4, elScreenElems, elScreenStretch);

  LayoutPad elPaddedScreen(&elScreen, 5, 5, 5, 5);

  uint16_t w, h;
  elPaddedScreen.draw(0, 0, display.width(), display.height(), &w, &h);
}
