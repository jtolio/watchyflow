#pragma once

#include "../../Watchy/WatchyApp.h"
#include "../Calendar/CalendarApp.h"

class ResetStepCounterApp : public WatchyApp {
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

class TriggerCalendarResetApp : public WatchyApp {
public:
  explicit TriggerCalendarResetApp(CalendarApp *cal) : cal_(cal) {}
  virtual AppState show(Watchy *watchy, Display *display,
                        bool partialRefresh) override {
    cal_->forceCacheMiss();
    watchy->triggerNetworkFetch();
    return APP_EXIT;
  }

private:
  CalendarApp *cal_;
};
