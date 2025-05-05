#pragma once

#include "../../Watchy/WatchyApp.h"

class StopwatchApp : public WatchyApp {
public:
  virtual void reset(Watchy *watchy) override;
  virtual AppState show(Watchy *watchy, Display *display,
                        bool partialRefresh) override;
  virtual void buttonUp(Watchy *watchy) override;
  virtual void buttonDown(Watchy *watchy) override;
  virtual AppState buttonSelect(Watchy *watchy) override;
  virtual AppState buttonBack(Watchy *watchy) override;
};
