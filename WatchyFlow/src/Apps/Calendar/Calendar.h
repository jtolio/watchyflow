#pragma once

#include "../../Layout/Layout.h"

class AlertsApp;

const uint8_t MAX_EVENT_NAME_LEN    = 24;
const uint8_t MAX_EVENTS_PER_COLUMN = 24;
const uint8_t MAX_ALARMS            = 24;
const uint8_t MAX_DAY_EVENTS        = 31;

typedef struct eventData {
  char summary[MAX_EVENT_NAME_LEN];
  time_t start;
  time_t end;
} eventData;

typedef struct dayEventsData {
  eventData events[MAX_DAY_EVENTS];
  uint8_t eventCount;
} dayEventsData;

typedef struct eventsData {
  eventData events[MAX_EVENTS_PER_COLUMN];
  uint8_t eventCount;
} eventsData;

typedef struct alarmData {
  char summary[MAX_EVENT_NAME_LEN];
  time_t start;
} alarmData;

typedef struct alarmsData {
  alarmData alarms[MAX_ALARMS];
  uint8_t alarmCount;
} alarmsData;

void reset(dayEventsData *data);
void reset(eventsData *data);
void reset(alarmsData *data);
void addEvent(dayEventsData *data, String summary, time_t start, time_t end);
void addEvent(eventsData *data, String summary, time_t start, time_t end);
void addAlarm(alarmsData *data, String summary, time_t start);

class CalendarDayEvents : public LayoutElement {
public:
  CalendarDayEvents(dayEventsData *data, Watchy *watchy, int32_t offsetSeconds,
                    uint16_t color)
      : data_(data), watchy_(watchy), offset_(offsetSeconds), color_(color) {}
  CalendarDayEvents(const CalendarDayEvents &copy)
      : data_(copy.data_), watchy_(copy.watchy_), offset_(copy.offset_),
        color_(copy.color_) {}

  void size(Display *display, uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override {
    maybeDraw(display, 0, 0, targetWidth, targetHeight, width, height, true);
  }

  void draw(Display *display, int16_t x0, int16_t y0, uint16_t targetWidth,
            uint16_t targetHeight, uint16_t *width, uint16_t *height) override {
    maybeDraw(display, x0, y0, targetWidth, targetHeight, width, height, false);
  }

  LayoutElement::ptr clone() const override {
    return std::make_shared<CalendarDayEvents>(*this);
  }

private:
  void maybeDraw(Display *display, int16_t x0, int16_t y0, uint16_t targetWidth,
                 uint16_t targetHeight, uint16_t *width, uint16_t *height,
                 bool noop);

private:
  dayEventsData *data_;
  Watchy *watchy_;
  int32_t offset_;
  uint16_t color_;
};

class CalendarMonth : public LayoutElement {
public:
  CalendarMonth(dayEventsData *data, Watchy *watchy, int32_t offsetEvents,
                bool dayDelta, uint16_t color)
      : data_(data), watchy_(watchy), offset_(offsetEvents),
        dayDelta_(dayDelta), color_(color) {}
  CalendarMonth(const CalendarMonth &copy)
      : data_(copy.data_), watchy_(copy.watchy_), offset_(copy.offset_),
        dayDelta_(copy.dayDelta_), color_(copy.color_) {}

  void size(Display *display, uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override {
    *width  = targetWidth;
    *height = targetHeight;
  }

  void draw(Display *display, int16_t x0, int16_t y0, uint16_t targetWidth,
            uint16_t targetHeight, uint16_t *width, uint16_t *height) override;

  LayoutElement::ptr clone() const override {
    return std::make_shared<CalendarMonth>(*this);
  }

private:
  dayEventsData *data_;
  Watchy *watchy_;
  int32_t offset_;
  bool dayDelta_;
  uint16_t color_;
};

class CalendarColumn : public LayoutElement {
public:
  CalendarColumn(eventsData *data, Watchy *watchy, int32_t offsetSeconds,
                 uint16_t color)
      : data_(data), watchy_(watchy), offset_(offsetSeconds), color_(color) {}
  CalendarColumn(const CalendarColumn &copy)
      : data_(copy.data_), watchy_(copy.watchy_), offset_(copy.offset_),
        color_(copy.color_) {}

  void size(Display *display, uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override {
    *width  = targetWidth;
    *height = targetHeight;
  }

  void draw(Display *display, int16_t x0, int16_t y0, uint16_t targetWidth,
            uint16_t targetHeight, uint16_t *width, uint16_t *height) override;

  LayoutElement::ptr clone() const override {
    return std::make_shared<CalendarColumn>(*this);
  }

  static bool shouldVibrateOnEventStart(Watchy *watchy, eventsData *data);

private:
  void resizeText(Display *display, char *text, uint8_t buflen, uint16_t width,
                  uint16_t height, int16_t *x1, int16_t *y1, uint16_t *tw,
                  uint16_t *th);

private:
  eventsData *data_;
  Watchy *watchy_;
  int32_t offset_;
  uint16_t color_;
};

class CalendarHourBar : public LayoutElement {
public:
  CalendarHourBar(Watchy *watchy, int32_t offsetSeconds, uint16_t color)
      : watchy_(watchy), offset_(offsetSeconds), color_(color) {}
  CalendarHourBar(const CalendarHourBar &copy)
      : watchy_(copy.watchy_), offset_(copy.offset_), color_(copy.color_) {}

  void size(Display *display, uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override {
    maybeDraw(display, 0, 0, targetWidth, targetHeight, width, height, true);
  }
  void draw(Display *display, int16_t x0, int16_t y0, uint16_t targetWidth,
            uint16_t targetHeight, uint16_t *width, uint16_t *height) override {
    maybeDraw(display, x0, y0, targetWidth, targetHeight, width, height, false);
  }

  LayoutElement::ptr clone() const override {
    return std::make_shared<CalendarHourBar>(*this);
  }

private:
  void maybeDraw(Display *display, int16_t x0, int16_t y0, uint16_t targetWidth,
                 uint16_t targetHeight, uint16_t *width, uint16_t *height,
                 bool noop);

private:
  Watchy *watchy_;
  int32_t offset_;
  uint16_t color_;
};

class CalendarAlarms : public LayoutElement {
public:
  CalendarAlarms(alarmsData *data, Watchy *watchy, uint16_t color)
      : data_(data), watchy_(watchy), color_(color) {}
  CalendarAlarms(const CalendarAlarms &copy)
      : data_(copy.data_), watchy_(copy.watchy_), color_(copy.color_) {}

  void size(Display *display, uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override {
    maybeDraw(display, 0, 0, targetWidth, targetHeight, width, height, true);
  }
  void draw(Display *display, int16_t x0, int16_t y0, uint16_t targetWidth,
            uint16_t targetHeight, uint16_t *width, uint16_t *height) override {
    maybeDraw(display, x0, y0, targetWidth, targetHeight, width, height, false);
  }

  LayoutElement::ptr clone() const override {
    return std::make_shared<CalendarAlarms>(*this);
  }

  static bool shouldVibrateOnEventStart(Watchy *watchy, alarmsData *data,
                                        AlertsApp *alerts);

private:
  void maybeDraw(Display *display, int16_t x0, int16_t y0, uint16_t targetWidth,
                 uint16_t targetHeight, uint16_t *width, uint16_t *height,
                 bool noop);

private:
  alarmsData *data_;
  Watchy *watchy_;
  uint16_t color_;
};
