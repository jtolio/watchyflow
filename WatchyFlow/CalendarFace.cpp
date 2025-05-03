#include "CalendarFace.h"

#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <Fonts/Picopixel.h>
#include "Layout.h"
#include "Battery.h"
#include "Calendar.h"
#include "Weather.h"
#include "Seven_Segment10pt7b.h"
#include "DSEG7_Classic_Bold_25.h"
#include "DSEG7_Classic_Regular_39.h"
#include "icons.h"

#define DARKMODE false

const uint16_t FOREGROUND_COLOR = DARKMODE ? GxEPD_WHITE : GxEPD_BLACK;
const uint16_t BACKGROUND_COLOR = DARKMODE ? GxEPD_BLACK : GxEPD_WHITE;

const uint8_t MAX_CALENDAR_COLUMNS                 = 6;
const uint16_t MAX_SECONDS_BETWEEN_WEATHER_UPDATES = 60 * 60 * 2;
const int32_t DAY_SCROLL_INCREMENT                 = 3 * 30 * 60;

RTC_DATA_ATTR dayEventsData calendarDay;
RTC_DATA_ATTR eventsData calendar[MAX_CALENDAR_COLUMNS];
RTC_DATA_ATTR alarmsData alarms;
RTC_DATA_ATTR uint8_t activeCalendarColumns;
RTC_DATA_ATTR char calendarError[32];
RTC_DATA_ATTR uint16_t lastTemperature;
RTC_DATA_ATTR int16_t weatherConditionCode;
RTC_DATA_ATTR int32_t dayScheduleOffset;
RTC_DATA_ATTR int32_t monthEventOffset;
RTC_DATA_ATTR bool viewShowAboveCalendar;
RTC_DATA_ATTR bool monthView;
RTC_DATA_ATTR bool monthDayAbs;

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
  lastTemperature       = 0;
  weatherConditionCode  = -1;
  dayScheduleOffset     = 0;
  viewShowAboveCalendar = true;
  monthView             = false;
  monthDayAbs           = false;
  monthEventOffset      = 0;
  zeroError();
}

bool CalendarFace::fetchNetwork(Watchy *watchy) {
  bool success = true;
  {
    HTTPClient http;
    http.setConnectTimeout(1000 * 10);
    http.setTimeout(1000 * 10);
    String calQueryURL = settings_.calendarAccountURL;
    if (forceCacheMiss_) {
      calQueryURL += "?force_cache_miss=true";
    }
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
      weatherConditionCode   = int(responseObject["weather"][0]["id"]);
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
    if (!event.hasOwnProperty("start")) {
      continue;
    }
    if (!event.hasOwnProperty("end")) {
      continue;
    }

    time_t start = (time_t)(long)event["start"];
    time_t end   = (time_t)(long)event["end"];

    String summary = event["summary"];
    bool allDay    = (bool)event["day"];
    if (allDay) {
      addEvent(&calendarDay, summary, start, end);
      continue;
    }

    if (summary.indexOf(String("[WATCHY ALARM]")) >= 0) {
      summary.replace(String("[WATCHY ALARM]"), String(""));
      summary.trim();
      addAlarm(&alarms, summary, start);
      continue;
    }

    if (!event.hasOwnProperty("column")) {
      continue;
    }
    int column = (int)event["column"];
    if (column >= activeCalendarColumns || column < 0) {
      continue;
    }

    addEvent(&calendar[column], summary, start, end);
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

bool CalendarFace::show(Watchy *watchy, Display *display, bool partialRefresh) {
  display->fillScreen(BACKGROUND_COLOR);
  display->setTextWrap(false);
  tmElements_t currentTime = watchy->localtime();

  uint16_t color = FOREGROUND_COLOR;

  int displayHour = ((currentTime.Hour + 11) % 12) + 1;
  String timeStr  = String(displayHour) + ":";
  if (currentTime.Minute < 10) {
    timeStr += "0";
  }
  timeStr += String(currentTime.Minute);

  tmElements_t offsetTime =
      watchy->toLocalTime(watchy->unixtime() + dayScheduleOffset);

  time_t lastSuccessfulFetch = watchy->lastSuccessfulNetworkFetch();
  bool weatherUpToDate =
      lastSuccessfulFetch + MAX_SECONDS_BETWEEN_WEATHER_UPDATES >=
      watchy->unixtime();

  String tempStr;
  if (weatherUpToDate) {
    tempStr = String(lastTemperature) + (settings_.metric ? "C" : "F");
  } else {
    if (settings_.metric) {
      tempStr = String(watchy->temperature()) + "C";
    } else {
      tempStr = String(watchy->temperature() * 9 / 5 + 32) + "F";
    }
  }

  String dayOfWeekStr  = dayShortStr(offsetTime.Wday);
  String monthStr      = monthShortStr(offsetTime.Month);
  String dayOfMonthStr = String(offsetTime.Day);
  if (offsetTime.Day < 10) {
    dayOfMonthStr = String("0") + dayOfMonthStr;
  }

  String errorMessage = calendarError;
  if (errorMessage.length() == 0) {
    if (lastSuccessfulFetch > 0) {
      time_t age   = watchy->unixtime() - lastSuccessfulFetch;
      errorMessage = secondsToReadable(age);
    }
  }

  LayoutText elemTemp(tempStr, &Seven_Segment10pt7b, color);
  LayoutBitmap elemWiFiOff(wifioff, 26, 18, color);
  LayoutElement *elemTempOrWiFi =
      weatherUpToDate ? static_cast<LayoutElement *>(&elemTemp)
                      : static_cast<LayoutElement *>(&elemWiFiOff);

  LayoutColumns elemSmallTop({
      LayoutCell(
          LayoutCenter(LayoutText(timeStr, &DSEG7_Classic_Bold_25, color))),
      LayoutCell(LayoutFill(), true),
      LayoutCell(LayoutCenter(*elemTempOrWiFi)),
      LayoutCell(LayoutSpacer(5)),
      LayoutCell(LayoutCenter(LayoutBattery(watchy, color))),
  });

  LayoutRows elemLargeTop({
      LayoutCell(LayoutColumns({
          LayoutCell(LayoutCenter(LayoutBitmap(steps, 19, 23, color))),
          LayoutCell(LayoutSpacer(5)),
          LayoutCell(LayoutCenter(LayoutText(String(watchy->stepCounter()),
                                             &Seven_Segment10pt7b, color))),
          LayoutCell(LayoutFill(), true),
          LayoutCell(
              LayoutCenter(LayoutText(String(watchy->battVoltage()) + "V",
                                      &Seven_Segment10pt7b, color))),
          LayoutCell(LayoutSpacer(5)),
          LayoutCell(LayoutCenter(LayoutBattery(watchy, color))),
      })),
      LayoutCell(LayoutSpacer(5)),
      LayoutCell(LayoutColumns({
          LayoutCell(LayoutSpacer(5)),
          LayoutCell(LayoutCenter(
              LayoutText(timeStr, &DSEG7_Classic_Regular_39, color))),
          LayoutCell(LayoutSpacer(5)),
          LayoutCell(LayoutFill(), true),
          LayoutCell(LayoutCenter(elemTemp)),
          LayoutCell(LayoutSpacer(5)),
          LayoutCell(LayoutCenter(
              LayoutWeatherIcon(weatherUpToDate, weatherConditionCode, color))),
      })),
  });

  LayoutElement *elemTop = viewShowAboveCalendar
                               ? static_cast<LayoutElement *>(&elemLargeTop)
                               : static_cast<LayoutElement *>(&elemSmallTop);

  LayoutBottomAlign elemError(LayoutRightAlign(LayoutBackground(
      LayoutPad(LayoutText(errorMessage, &Picopixel, color), 2, 2, 2, 2),
      BACKGROUND_COLOR)));

  std::vector<LayoutCell> calColumns;
  calColumns.reserve(activeCalendarColumns + 1);
  calColumns.push_back(
      LayoutCell(CalendarHourBar(watchy, dayScheduleOffset, color)));
  for (int i = 0; i < activeCalendarColumns; i++) {
    calColumns.push_back(LayoutCell(
        CalendarColumn(&calendar[i], watchy, dayScheduleOffset, color), true));
  }

  LayoutRows elemCalendarDay({
      LayoutCell(
          CalendarDayEvents(&calendarDay, watchy, dayScheduleOffset, color)),
      LayoutCell(LayoutColumns(calColumns), true),
  });

  CalendarMonth elemCalendarMonth(&calendarDay, watchy, monthEventOffset,
                                  !monthDayAbs, color);

  LayoutElement *elemCalendar =
      monthView ? static_cast<LayoutElement *>(&elemCalendarMonth)
                : static_cast<LayoutElement *>(&elemCalendarDay);

  uint16_t w, h;
  LayoutRows(
      {
          LayoutCell(*elemTop),
          LayoutCell(LayoutSpacer(5)),
          LayoutCell(LayoutColumns({
                         LayoutCell(LayoutRows({
                             LayoutCell(LayoutRotate(
                                 LayoutText(dayOfWeekStr + " " + monthStr +
                                                " " + dayOfMonthStr,
                                            &Seven_Segment10pt7b, color),
                                 3)),
                             LayoutCell(LayoutFill(), true),
                         })),
                         LayoutCell(LayoutSpacer(5)),
                         LayoutCell(LayoutBorder(
                                        LayoutOverlay(*elemCalendar, elemError),
                                        true, false, true, true, color),
                                    true),
                     }),
                     true),
          LayoutCell(CalendarAlarms(&alarms, watchy, color)),
      })
      .draw(display, 0, 0, display->width(), display->height(), &w, &h);

  display->display(partialRefresh);

  if (currentTime.Hour == 0 && currentTime.Minute == 0) {
    watchy->resetStepCounter();
  }

  return true;
}

void CalendarFace::buttonDown(Watchy *watchy) {
  if (monthView) {
    monthDayAbs = !monthDayAbs;
    return;
  }
  if (viewShowAboveCalendar) {
    viewShowAboveCalendar = false;
    return;
  }
  dayScheduleOffset += DAY_SCROLL_INCREMENT;
  if (dayScheduleOffset >= (36 - 6) * 60 * 60) {
    dayScheduleOffset = (36 - 6) * 60 * 60;
  }
}

void CalendarFace::buttonUp(Watchy *watchy) {
  if (monthView || dayScheduleOffset == 0) {
    viewShowAboveCalendar = !viewShowAboveCalendar;
    return;
  }
  dayScheduleOffset -= DAY_SCROLL_INCREMENT;
  if (dayScheduleOffset <= 0) {
    dayScheduleOffset = 0;
  }
}

bool CalendarFace::buttonBack(Watchy *watchy) {
  if (dayScheduleOffset == 0) {
    monthView = !monthView;
  } else {
    viewShowAboveCalendar = true;
  }
  dayScheduleOffset = 0;
  monthEventOffset  = 0;
  return false;
}
