#include "CalendarFace.h"

#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <Fonts/Picopixel.h>
#include "Layout.h"
#include "Battery.h"
#include "Calendar.h"
#include "Seven_Segment10pt7b.h"
#include "DSEG7_Classic_Bold_25.h"
#include "icons.h"

#define DARKMODE false

const uint16_t FOREGROUND_COLOR = DARKMODE ? GxEPD_WHITE : GxEPD_BLACK;
const uint16_t BACKGROUND_COLOR = DARKMODE ? GxEPD_BLACK : GxEPD_WHITE;

const uint8_t MAX_CALENDAR_COLUMNS                 = 8;
const uint16_t MAX_SECONDS_BETWEEN_WEATHER_UPDATES = 60 * 60 * 2;

RTC_DATA_ATTR dayEventsData calendarDay;
RTC_DATA_ATTR eventsData calendar[MAX_CALENDAR_COLUMNS];
RTC_DATA_ATTR alarmsData alarms;
RTC_DATA_ATTR uint8_t activeCalendarColumns;
RTC_DATA_ATTR char calendarError[32];
RTC_DATA_ATTR time_t lastCalendarFetch;
RTC_DATA_ATTR time_t lastWeatherFetch;
RTC_DATA_ATTR uint16_t lastTemperature;

void zeroError() {
  for (int i = 0; i < (sizeof(calendarError) / sizeof(calendarError[0])); i++) {
    calendarError[i] = 0;
  }
}

void CalendarFace::reset(Watchy *watchy) {
  activeCalendarColumns = 1;
  ::reset(&calendar[0]);
  ::reset(&alarms);
  ::reset(&calendarDay);
  lastCalendarFetch = 0;
  lastWeatherFetch  = 0;
  lastTemperature   = 0;
  zeroError();
}

void CalendarFace::buttonBack(Watchy *watchy) {
  watchy->triggerNetworkFetch();
  lastCalendarFetch = 0;
  lastWeatherFetch  = 0;
}

bool CalendarFace::fetchNetwork(Watchy *watchy) {
  bool success = true;
  {
    HTTPClient http;
    http.setConnectTimeout(1000 * 10);
    http.setTimeout(1000 * 10);
    String calQueryURL = settings_.calendarAccountURL;
    http.begin(calQueryURL.c_str());
    int httpResponseCode = http.GET();
    if (httpResponseCode == 200) {
      zeroError();
      parseCalendar(watchy, http.getString());
    } else {
      String error(httpResponseCode);
      error.toCharArray(calendarError,
                        sizeof(calendarError) / sizeof(calendarError[0]));
      success = false;
    }
    http.end();
  }

  {
    HTTPClient http;
    http.setConnectTimeout(1000 * 10);
    http.setTimeout(1000 * 10);
    String weatherQueryURL = settings_.weatherURL;
    http.begin(weatherQueryURL.c_str());
    if (http.GET() == 200) {
      String payload         = http.getString();
      JSONVar responseObject = JSON.parse(payload);
      lastTemperature        = int(responseObject["main"]["temp"]);
      lastWeatherFetch       = unixEpochTime(watchy->time());
      watchy->setTimezoneOffset(int(responseObject["timezone"]));
    } else {
      success = false;
    }
    http.end();
  }

  return success;
}

void CalendarFace::parseCalendar(Watchy *watchy, String payload) {
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

  lastCalendarFetch     = unixEpochTime(watchy->time());
  activeCalendarColumns = (uint8_t)(int)parsed["columns"];
  if (activeCalendarColumns >= MAX_CALENDAR_COLUMNS) {
    activeCalendarColumns = MAX_CALENDAR_COLUMNS;
  }
  if (activeCalendarColumns <= 0) {
    activeCalendarColumns = 1;
  }
  for (int i = 0; i < activeCalendarColumns; i++) {
    ::reset(&calendar[i]);
  }
  ::reset(&calendarDay);
  ::reset(&alarms);

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

String secondsToReadable(time_t val) {
  if (val < 60 && val > -60) {
    return String(val) + "s";
  }
  val /= 60;
  if (val < 60 && val > -60) {
    return String(val) + "m";
  }
  val /= 60;
  if (val < 24 && val > -24) {
    return String(val) + "h";
  }
  val /= 24;
  return String(val) + "d";
}

bool CalendarFace::show(Watchy *watchy, Display *display) {
  display->fillScreen(BACKGROUND_COLOR);
  display->setTextWrap(false);
  tmElements_t currentTime = watchy->time();

  uint16_t color = FOREGROUND_COLOR;

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

  String weatherStr = String(lastTemperature) + (settings_.metric ? "C" : "F");

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

  LayoutBitmap elWifi(wifioff, 26, 18, color);
  LayoutText elTemp(weatherStr, &Seven_Segment10pt7b, color);
  LayoutElement *elTempOrWifi = &elWifi;
  if (lastWeatherFetch + MAX_SECONDS_BETWEEN_WEATHER_UPDATES >=
      unixEpochTime(currentTime)) {
    elTempOrWifi = &elTemp;
  }
  LayoutCenter elTempOrWifiCentered(elTempOrWifi);

  LayoutBattery elBattery(watchy->battVoltage(), color);
  LayoutCenter elBatteryCentered(&elBattery);

  LayoutElement *elTopElems[] = {&elTimeCentered, &elFill,
                                 &elTempOrWifiCentered, &elSpacer,
                                 &elBatteryCentered};
  bool elTopStretch[]         = {false, true, false, false, false};
  LayoutColumns elTop(5, elTopElems, elTopStretch);

  String errorMessage = calendarError;
  if (errorMessage.length() == 0) {
    time_t oldestFetch = lastCalendarFetch;
    if (oldestFetch > lastWeatherFetch) {
      oldestFetch = lastWeatherFetch;
    }
    time_t age = unixEpochTime(currentTime) - oldestFetch;
    if (oldestFetch <= 0) {
      age = -24 * 60 * 60;
    }
    errorMessage = secondsToReadable(age);
  }
  LayoutText elError(errorMessage, &Picopixel, color);
  LayoutCenter elErrorCenter(&elError);

  LayoutText elDateWords(dayOfWeekStr + " " + monthStr + " " + dayOfMonthStr,
                         &Seven_Segment10pt7b, color);
  LayoutRotate elDateRotated(&elDateWords, 3);
  LayoutElement *elDateElems[] = {&elDateRotated, &elFill, &elErrorCenter};
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
    elCals[i]              = CalendarColumn(&calendar[i], watchy, color);
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

  CalendarAlarms elAlarms(&alarms, watchy, color);

  LayoutElement *elScreenElems[] = {&elTop, &elSpacer, &elMain, &elAlarms};
  bool elScreenStretch[]         = {false, false, true, false};
  LayoutRows elScreen(4, elScreenElems, elScreenStretch);

  LayoutPad elPaddedScreen(&elScreen, 5, 5, 5, 5);

  uint16_t w, h;
  elPaddedScreen.draw(display, 0, 0, display->width(), display->height(), &w,
                      &h);
  return true;
}
