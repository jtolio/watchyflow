#ifndef WATCHY_H
#define WATCHY_H

#include <TimeLib.h>
#include <GxEPD2_BW.h>
#include "Display.h"

class WatchyApp;

class Watchy {
public:
  static void wakeup(WatchyApp *app);

  GxEPD2_BW<WatchyDisplay, WatchyDisplay::HEIGHT> *display();
  tmElements_t time() { return time_; }

protected:
  Watchy(const tmElements_t &currentTime) : time_(currentTime) {}

private:
  tmElements_t time_;
};

class WatchyApp {
public:
  virtual bool show(Watchy *watchy) = 0;
  virtual bool fetchNetwork(Watchy *watchy) { return true; }

  virtual void reset() {}
  virtual void buttonUp() {}
  virtual void buttonDown() {}
  virtual void buttonSelect() {}
  virtual void buttonBack() {}

  virtual ~WatchyApp() = default;
};

#endif
