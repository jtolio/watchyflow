#pragma once

#include "../../Watchy/WatchyApp.h"
#include "../Alerts/AlertsApp.h"

class TimerApp : public WatchyApp {
public:
  TimerApp() : alerts_(NULL) {}
  explicit TimerApp(AlertsApp *alerts) : alerts_(alerts) {}

  void reset(Watchy *watchy) override;
  AppState show(Watchy *watchy, Display *display) override;
  void tick(Watchy *watchy) override;

  void buttonUp(Watchy *watchy) override;
  void buttonDown(Watchy *watchy) override;
  AppState buttonSelect(Watchy *watchy) override;
  AppState buttonBack(Watchy *watchy) override;

private:
  AlertsApp *alerts_;
};
