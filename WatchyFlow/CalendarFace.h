#ifndef CALENDAR_FACE_H
#define CALENDAR_FACE_H

#include "Watchy.h"

typedef struct CalendarSettings {
  String calendarAccountURL;
  bool metric;
  String weatherURL;
} CalendarSettings;

class CalendarFace : public WatchyApp {
public:
  explicit CalendarFace(CalendarSettings settings)
      : settings_(settings), forceCacheMiss_(false) {}

  bool show(Watchy *watchy, Display *display, bool partialRefresh) override;
  bool fetchNetwork(Watchy *watchy) override;

  void reset(Watchy *watchy) override;

  void buttonUp(Watchy *watchy) override;
  void buttonDown(Watchy *watchy) override;
  bool buttonBack(Watchy *watchy) override;

  void forceCacheMiss() { forceCacheMiss_ = true; }

private:
  void parseCalendar(Watchy *watchy, String payload);

private:
  CalendarSettings settings_;
  bool forceCacheMiss_;
};

#endif
