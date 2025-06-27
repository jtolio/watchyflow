#pragma once

#include "../../Watchy/WatchyApp.h"
#include "../Calendar/CalendarApp.h"

class ResetStepCounterApp : public WatchyApp {
public:
  virtual AppState show(Watchy *watchy, Display *display) override {
    watchy->resetStepCounter();
    return APP_EXIT;
  }
};

class TriggerNetworkFetchApp : public WatchyApp {
public:
  virtual AppState show(Watchy *watchy, Display *display) override {
    watchy->triggerNetworkFetch();
    return APP_EXIT;
  }
};

class TriggerCalendarResetApp : public WatchyApp {
public:
  explicit TriggerCalendarResetApp(CalendarApp *cal) : cal_(cal) {}
  virtual AppState show(Watchy *watchy, Display *display) override {
    cal_->forceCacheMiss();
    watchy->triggerNetworkFetch();
    return APP_EXIT;
  }

private:
  CalendarApp *cal_;
};

class SetCalendarLocationApp : public WatchyApp {
public:
  SetCalendarLocationApp() : cal_(NULL), loc_(0) {}
  SetCalendarLocationApp(const SetCalendarLocationApp &copy)
      : cal_(copy.cal_), loc_(copy.loc_) {}

  explicit SetCalendarLocationApp(CalendarApp *cal, int loc)
      : cal_(cal), loc_(loc) {}
  virtual AppState show(Watchy *watchy, Display *display) override {
    if (cal_ != NULL) {
      cal_->setActiveLocation(loc_);
    }
    watchy->triggerNetworkFetch();
    return APP_EXIT;
  }

private:
  CalendarApp *cal_;
  int loc_;
};
