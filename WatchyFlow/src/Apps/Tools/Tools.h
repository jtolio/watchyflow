#pragma once

#include "../../Watchy/WatchyApp.h"
#include "../Calendar/CalendarFace.h"

class ResetStepCounter : public WatchyApp {
public:
  virtual AppState show(Watchy *watchy, Display *display,
                        bool partialRefresh) override {
    watchy->resetStepCounter();
    return APP_EXIT;
  }
};

class TriggerNetworkFetchApp : public WatchyApp {
public:
  virtual AppState show(Watchy *watchy, Display *display,
                        bool partialRefresh) override {
    watchy->triggerNetworkFetch();
    return APP_EXIT;
  }
};

class TriggerCalendarReset : public WatchyApp {
public:
  explicit TriggerCalendarReset(CalendarFace *cal) : cal_(cal) {}
  virtual AppState show(Watchy *watchy, Display *display,
                        bool partialRefresh) override {
    cal_->forceCacheMiss();
    watchy->triggerNetworkFetch();
    return APP_EXIT;
  }

private:
  CalendarFace *cal_;
};
