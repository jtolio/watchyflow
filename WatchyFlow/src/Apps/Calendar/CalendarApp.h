#pragma once

#include "../../Watchy/WatchyApp.h"

// see settings.h.example for documentation and an example
typedef struct CalendarSettings {
  String calendarAccountURL;
  bool metric;
  String weatherURL;
  int8_t silenceWindowHourStart;
  int8_t silenceWindowHourEnd;
} CalendarSettings;

class CalendarApp : public WatchyApp {
public:
  explicit CalendarApp(CalendarSettings settings)
      : settings_(settings), forceCacheMiss_(false) {}

  AppState show(Watchy *watchy, Display *display, bool partialRefresh) override;
  FetchState fetchNetwork(Watchy *watchy) override;
  void tick(Watchy *watchy) override;

  void reset(Watchy *watchy) override;

  void buttonUp(Watchy *watchy) override;
  void buttonDown(Watchy *watchy) override;
  AppState buttonBack(Watchy *watchy) override;

  // if set, the next network fetch will tell the calendar server to clear
  // the server's cache.
  void forceCacheMiss() { forceCacheMiss_ = true; }

private:
  void parseCalendar(Watchy *watchy, String payload);

private:
  CalendarSettings settings_;
  bool forceCacheMiss_;
};
