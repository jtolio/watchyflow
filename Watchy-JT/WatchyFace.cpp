#include "WatchyFace.h"
#include "Layout.h"
#include <Fonts/Picopixel.h>

#define DARKMODE false

const uint8_t BATTERY_SEGMENT_WIDTH = 7;
const uint8_t BATTERY_SEGMENT_HEIGHT = 11;
const uint8_t BATTERY_SEGMENT_SPACING = 9;
const uint8_t WEATHER_ICON_WIDTH = 48;
const uint8_t WEATHER_ICON_HEIGHT = 32;

const uint8_t MAX_CALENDAR_COLUMNS = 8;
const uint8_t MAX_EVENT_NAME_LEN = 24;
const uint8_t MAX_EVENTS_PER_COLUMN = 16;
const uint8_t MAX_DAY_EVENTS = 5;

const GFXfont *SMALL_FONT = &Picopixel;

class LayoutBattery : public LayoutElement {
public:
  explicit LayoutBattery(float vbat) : vbat_(vbat) {}

  void size(uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override {
    *width = 37;
    *height = 21;
  }

  void draw(int16_t x0, int16_t y0,
            uint16_t targetWidth, uint16_t targetHeight,
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

time_t unixEpochTime(tmElements_t tm) {
  // the system clock is stored in the local timezone and not UTC, like most
  // unix systems.
  // the unix timestamp calculation is therefore off by the local timezone
  // offset from UTC, so to fix it we need to subtract the gmtOffset.
  // note that time_t may be 32 bits, and may have a Y2038 problem. it may
  // only make sense to use time_t for time deltas.
  return 1742221800 + 20*60;
  return makeTime(tm) - gmtOffset;
}

typedef struct dayEventData {
  char summary[MAX_EVENT_NAME_LEN];
  char start[11];
  char end[11];
} dayEventData;

typedef struct dayEventsData {
  dayEventData events[MAX_DAY_EVENTS];
  uint8_t eventCount;
} dayEventsData;

void reset(dayEventsData *data) {
  data->eventCount = 0;
}

void addEvent(dayEventsData *data, String summary, String start, String end) {
  if (data->eventCount >= MAX_DAY_EVENTS) { return; }
  if (data->eventCount == MAX_DAY_EVENTS - 1) { summary = "TOO MANY EVENTS"; }
  start.toCharArray(data->events[data->eventCount].start, 11);
  end.toCharArray(data->events[data->eventCount].end, 11);
  summary.toCharArray(data->events[data->eventCount].summary, MAX_EVENT_NAME_LEN);
  data->eventCount++;
}

class CalendarDayEvents : public LayoutElement {
public:
  CalendarDayEvents(dayEventsData *data, tmElements_t currentTime)
    : data_(data), currentTime_(currentTime) {}

  void size(uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override {
    *width = targetWidth;
    *height = 0;

    Watchy::Watchy::display.setFont(SMALL_FONT);

    String today = String(tmYearToCalendar(currentTime_.Year));
    today += (currentTime_.Month < 10) ? "-0" : "-";
    today += String(currentTime_.Month);
    today += (currentTime_.Day < 10) ? "-0" : "-";
    today += String(currentTime_.Day);

    for (int i = 0; i < data_->eventCount; i++) {
      if (String(data_->events[i].start) > today ||
          String(data_->events[i].end) <= today) {
        continue;
      }

      uint16_t textWidth, textHeight;
      int16_t x1, y1;
      Watchy::Watchy::display.getTextBounds(
        String(data_->events[0].summary), 0, 0,
        &x1, &y1, &textWidth, &textHeight);
      textWidth += 4;
      textHeight += 2;
      if (textWidth > *width) {
        *width = textWidth;
      }
      *height += textHeight;
    }
    if (*height > 0) { *height += 2; }
  }

  void draw(int16_t x0, int16_t y0,
            uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override {
    uint16_t color = DARKMODE ? GxEPD_WHITE : GxEPD_BLACK;
    *width = targetWidth;
    *height = 0;

    Watchy::Watchy::display.setFont(SMALL_FONT);
    Watchy::Watchy::display.setTextColor(color);

    String today = String(currentTime_.Year);
    today += (currentTime_.Month < 10) ? "-0" : "-";
    today += String(currentTime_.Month);
    today += (currentTime_.Day < 10) ? "-0" : "-";
    today += String(currentTime_.Day);

    for (int i = 0; i < data_->eventCount; i++) {
      if (String(data_->events[i].start) > today ||
          String(data_->events[i].end) <= today) {
        // TODO: fix day checking.
        // continue;
      }

      String text = data_->events[0].summary;
      uint16_t textWidth, textHeight;
      int16_t x1, y1;
      Watchy::Watchy::display.getTextBounds(
        text, 0, 0, &x1, &y1, &textWidth, &textHeight);
      textWidth += 4;
      textHeight += 2;
      x1 -= 2;
      y1 -= 2;
      if (textWidth > *width) {
        *width = textWidth;
      }
      Watchy::Watchy::display.setCursor(x0 - x1, y0 - y1 + *height);
      Watchy::Watchy::display.print(text);
      *height += textHeight;
    }

    if (*height > 0) {
      *height += 2;
      Watchy::Watchy::display.drawFastHLine(x0, y0 + *height, *width, color);
    }
  }
private:
  dayEventsData *data_;
  tmElements_t currentTime_;
};

typedef struct eventData {
  char summary[MAX_EVENT_NAME_LEN];
  time_t start;
  time_t end;
} eventData;

typedef struct eventsData {
  eventData events[MAX_EVENTS_PER_COLUMN];
  uint8_t eventCount;
} eventsData;

void reset(eventsData *data) {
  data->eventCount = 0;
}

void addEvent(eventsData *data, String summary, time_t start, time_t end) {
  if (data->eventCount >= MAX_EVENTS_PER_COLUMN) { return; }
  if (data->eventCount == MAX_EVENT_NAME_LEN - 1) { summary = "TOO MANY EVENTS"; }
  data->events[data->eventCount].start = start;
  data->events[data->eventCount].end = end;
  summary.toCharArray(data->events[data->eventCount].summary, MAX_EVENT_NAME_LEN);
  data->eventCount++;
}

class CalendarColumn : public LayoutElement {
public:
  CalendarColumn() {}
  CalendarColumn(eventsData *data, tmElements_t currentTime)
    : data_(data), currentTime_(currentTime) {}

  void size(uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override {
    *width = targetWidth;
    *height = targetHeight;
  }

  void draw(int16_t x0, int16_t y0,
            uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override {
    uint16_t color = DARKMODE ? GxEPD_WHITE : GxEPD_BLACK;
    Watchy::Watchy::display.setFont(SMALL_FONT);
    Watchy::Watchy::display.setTextColor(color);

    *width = targetWidth;
    *height = targetHeight;
    time_t now = unixEpochTime(currentTime_);
    time_t windowStart = now - (60 * 60);
    time_t windowEnd = now + (4 * 60 * 60);
    time_t secondsPerPixel = (windowEnd - windowStart) / targetHeight;

    Watchy::Watchy::display.drawFastHLine(x0 + (targetWidth / 2), y0 + ((now - windowStart) / secondsPerPixel), targetWidth / 2, color);

    for (int i = 0; i < data_->eventCount; i++) {
      eventData *event = &(data_->events[i]);
      time_t eventStart = event->start;
      time_t eventEnd = event->end;
      if (eventStart < windowStart) {
        eventStart = windowStart;
      } else {
        Watchy::Watchy::display.drawFastHLine(x0, y0 + ((eventStart - windowStart) / secondsPerPixel), targetWidth, color);
      }
      int16_t eventOffset = (eventStart - windowStart) / secondsPerPixel;
      int16_t eventSize = (eventEnd - eventStart) / secondsPerPixel;
      if (eventEnd > windowEnd) {
        eventEnd = windowEnd;
      } else {
        Watchy::Watchy::display.drawFastHLine(x0, y0 + ((eventEnd - windowStart) / secondsPerPixel), targetWidth, color);
      }
      Watchy::Watchy::display.drawFastVLine(
        x0, y0 + eventOffset, eventSize, color);
      Watchy::Watchy::display.drawFastVLine(
        x0 + targetWidth - 1, y0 + eventOffset, eventSize, color);
      String summary = event->summary;
      int16_t x1, y1;
      uint16_t tw, th;
      Watchy::Watchy::display.getTextBounds(summary, 0, 0, &x1, &y1, &tw, &th);
      Watchy::Watchy::display.setCursor(x0 - x1 + 2, y0 - y1 + eventOffset + 2) ;
      Watchy::Watchy::display.print(summary);
    }
  }
private:
  eventsData *data_;
  tmElements_t currentTime_;
};

RTC_DATA_ATTR dayEventsData calendarDay;
RTC_DATA_ATTR eventsData calendar[MAX_CALENDAR_COLUMNS];
RTC_DATA_ATTR uint8_t activeCalendarColumns = 0;

void WatchyFace::hourlyUpdate() {
  const char *payload = "{\"status\": \"ok\", \"columns\": 2, \"events\": [{\"start\": \"2025-03-17\", \"end\": \"2025-03-18\", \"summary\": \"Sam's Birthday\", \"day\": true, \"column\": -1}, {\"start\": \"2025-03-17T08:30:00-04:00\", \"end\": \"2025-03-17T14:30:00-04:00\", \"summary\": \"claire at tch\", \"day\": false, \"start-unix\": 1742214600, \"end-unix\": 1742236200, \"column\": 0}, {\"start\": \"2025-03-17T09:45:00-04:00\", \"end\": \"2025-03-17T10:15:00-04:00\", \"summary\": \"Team Satellite Standup\", \"day\": false, \"start-unix\": 1742219100, \"end-unix\": 1742220900, \"column\": 1}, {\"start\": \"2025-03-17T10:30:00-04:00\", \"end\": \"2025-03-17T11:30:00-04:00\", \"summary\": \"Product/Engineering Staff meeting\", \"day\": false, \"start-unix\": 1742221800, \"end-unix\": 1742225400, \"column\": 1}, {\"start\": \"2025-03-17T11:30:00-04:00\", \"end\": \"2025-03-17T12:00:00-04:00\", \"summary\": \"Storj Select - Standup\", \"day\": false, \"start-unix\": 1742225400, \"end-unix\": 1742227200, \"column\": 1}, {\"start\": \"2025-03-17T12:00:00-04:00\", \"end\": \"2025-03-17T12:30:00-04:00\", \"summary\": \"Kaloyan / JT\", \"day\": false, \"start-unix\": 1742227200, \"end-unix\": 1742229000, \"column\": 1}, {\"start\": \"2025-03-17T12:30:00-04:00\", \"end\": \"2025-03-17T13:00:00-04:00\", \"summary\": \"Ethan / JT\", \"day\": false, \"start-unix\": 1742229000, \"end-unix\": 1742230800, \"column\": 1}, {\"start\": \"2025-03-17T13:00:00-04:00\", \"end\": \"2025-03-17T13:30:00-04:00\", \"summary\": \"Jeff / JT\", \"day\": false, \"start-unix\": 1742230800, \"end-unix\": 1742232600, \"column\": 1}, {\"start\": \"2025-03-17T14:00:00-04:00\", \"end\": \"2025-03-17T15:00:00-04:00\", \"summary\": \"Sync \", \"day\": false, \"start-unix\": 1742234400, \"end-unix\": 1742238000, \"column\": 1}, {\"start\": \"2025-03-17T15:00:00-04:00\", \"end\": \"2025-03-17T16:20:00-04:00\", \"summary\": \"Executive Staff Meeting \", \"day\": false, \"start-unix\": 1742238000, \"end-unix\": 1742242800, \"column\": 0}, {\"start\": \"2025-03-17T16:30:00-04:00\", \"end\": \"2025-03-17T17:00:00-04:00\", \"summary\": \"no meeting wrap up\", \"day\": false, \"start-unix\": 1742243400, \"end-unix\": 1742245200, \"column\": 1}]}";

  JSONVar parsed = JSON.parse(payload);
  if (!parsed.hasOwnProperty("status")) { return; }
  if (!parsed.hasOwnProperty("columns")) { return; }
  if (!parsed.hasOwnProperty("events")) { return; }
  String status = parsed["status"];
  if (status != "ok") {
    return;
  }

  activeCalendarColumns = (uint8_t)(int)parsed["columns"];
  if (activeCalendarColumns >= MAX_CALENDAR_COLUMNS) {
    activeCalendarColumns = MAX_CALENDAR_COLUMNS;
  }
  if (activeCalendarColumns <= 0) { activeCalendarColumns = 1; }
  for (int i = 0; i < activeCalendarColumns; i++) {
    reset(&calendar[i]);
  }
  reset(&calendarDay);

  JSONVar events = parsed["events"];
  for (int i = 0; i  < events.length(); i++) {
    JSONVar event = events[i];
    if (!event.hasOwnProperty("summary")) { continue; }
    if (!event.hasOwnProperty("day")) { continue; }
    if (!event.hasOwnProperty("column")) { continue; }
    if (!event.hasOwnProperty("start")) { continue; }
    if (!event.hasOwnProperty("end")) { continue; }

    String summary = event["summary"];
    bool allDay = (bool)event["day"];
    int column = (int)event["column"];
    if (column >= activeCalendarColumns) {
      continue;
    }
    String start = event["start"];
    String end = event["end"];
    if (allDay || column < 0) {
      addEvent(&calendarDay, summary, start, end);
      continue;
    }
    if (!event.hasOwnProperty("start-unix")) { continue; }
    if (!event.hasOwnProperty("end-unix")) { continue; }
    time_t startUnix = (time_t)(long)event["start-unix"];
    time_t endUnix = (time_t)(long)event["end-unix"];
    addEvent(&calendar[column], summary, startUnix, endUnix);
  }
}

void WatchyFace::drawWatchFace() {
  if (activeCalendarColumns <= 0) { hourlyUpdate(); }
  display.fillScreen(DARKMODE ? GxEPD_BLACK : GxEPD_WHITE);
  display.setTextWrap(false);

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

  LayoutBattery elBattery(getBatteryVoltage());
  LayoutCenter elBatteryCentered(&elBattery);

  LayoutElement *elTopElems[] = {&elTimeCentered, &elFill, &elTempOrWifiCentered, &elSpacer, &elBatteryCentered};
  bool elTopStretch[] = {false, true, false, false, false};
  LayoutColumns elTop(5, elTopElems, elTopStretch);

  LayoutText elDateWords(dayOfWeekStr + ", " + monthStr + " " +
      dayOfMonthStr, &Seven_Segment10pt7b, color);
  LayoutRotate elDateRotated(&elDateWords, 3);
  LayoutElement *elDateElems[] = {&elDateRotated, &elFill};
  bool elDateStretch[] = {false, true};
  LayoutRows elDate(2, elDateElems, elDateStretch);

  CalendarDayEvents elCalendarDay(&calendarDay, currentTime);

  CalendarColumn elCals[MAX_CALENDAR_COLUMNS];
  LayoutElement *elCalColElems[MAX_CALENDAR_COLUMNS];
  bool elCalColStretch[MAX_CALENDAR_COLUMNS];
  for (int i = 0; i < MAX_CALENDAR_COLUMNS; i++) {
    elCals[i] = CalendarColumn(&calendar[i], currentTime);
    elCalColElems[i] = &elCals[i];
    elCalColStretch[i] = true;
  }
  LayoutColumns elCalColumns(activeCalendarColumns, elCalColElems,
                             elCalColStretch);

  LayoutElement *elCalendarPartsElems[] = {&elCalendarDay, &elCalColumns};
  bool elCalendarPartsStretch[] = {false, true};
  LayoutRows elCalendarParts(2, elCalendarPartsElems, elCalendarPartsStretch);

  LayoutBorder elCalendarBorder(&elCalendarParts, true, false, false, true, color);

  LayoutElement *elMainElems[] = {&elDate, &elSpacer, &elCalendarBorder};
  bool elMainStretch[] = {false, false, true};
  LayoutColumns elMain(3, elMainElems, elMainStretch);

  LayoutElement *elScreenElems[] = {&elTop, &elSpacer, &elMain};
  bool elScreenStretch[] = {false, false, true};
  LayoutRows elScreen(3, elScreenElems, elScreenStretch);

  LayoutPad elPaddedScreen(&elScreen, 5, 5, 5, 5);

  uint16_t w, h;
  elPaddedScreen.draw(0, 0, display.width(), display.height(), &w, &h);
}
