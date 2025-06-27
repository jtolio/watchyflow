#pragma once

#include "../../Watchy/WatchyApp.h"

class AboutApp : public WatchyApp {
public:
  void reset(Watchy *watchy) override;
  AppState show(Watchy *watchy, Display *display) override;

  static void presleep();
};
