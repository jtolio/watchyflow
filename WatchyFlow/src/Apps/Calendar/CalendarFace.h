#pragma once

#include "../../Watchy/WatchyApp.h"

typedef struct CalendarSettings {
  String calendarAccountURL;
  bool metric;
  String weatherURL;
} CalendarSettings;

class CalendarFace : public WatchyApp {
public:
  explicit CalendarFace(CalendarSettings settings)
      : settings_(settings), forceCacheMiss_(false) {}

  AppState show(Watchy *watchy, Display *display, bool partialRefresh) override;
  FetchState fetchNetwork(Watchy *watchy) override;

  void reset(Watchy *watchy) override;

  void buttonUp(Watchy *watchy) override;
  void buttonDown(Watchy *watchy) override;
  AppState buttonBack(Watchy *watchy) override;

  void forceCacheMiss() { forceCacheMiss_ = true; }

  void tick(Watchy *watchy) override;

private:
  void parseCalendar(Watchy *watchy, String payload);

private:
  CalendarSettings settings_;
  bool forceCacheMiss_;
};
