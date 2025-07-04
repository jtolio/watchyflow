#pragma once

#include "../../Watchy/WatchyApp.h"

class StopwatchApp : public WatchyApp {
public:
  void reset(Watchy *watchy) override;
  AppState show(Watchy *watchy, Display *display) override;

  void buttonUp(Watchy *watchy) override;
  void buttonDown(Watchy *watchy) override;
  AppState buttonSelect(Watchy *watchy) override;
  AppState buttonBack(Watchy *watchy) override;
};
