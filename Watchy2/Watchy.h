#ifndef WATCHY_H
#define WATCHY_H

#include <TimeLib.h>
#include <GxEPD2_BW.h>
#include "Display.h"

class WatchyApp;

typedef GxEPD2_BW<WatchyDisplay, WatchyDisplay::HEIGHT> Display;

typedef struct WatchySettings {
  // number of seconds between network fetch attempts
  int networkFetchIntervalSeconds;
  // number of failing network fetch tries before giving up until the next
  // interval.
  int networkFetchTries;

  // WiFi settings (TODO, support more than one)
  String wifiSSID;
  String wifiPass;
} WatchySettings;

class Watchy {
public:
  static void wakeup(WatchyApp *app, WatchySettings settings);
  static void sleep();

public:
  tmElements_t time() { return time_; }
  void vibrate(uint8_t intervalMs = 100, uint8_t length = 20);
  float battVoltage();

  // TODO
  void setTimezoneOffset(time_t offset) {}

  void triggerNetworkFetch();

protected:
  Watchy(const tmElements_t &currentTime) : time_(currentTime) {}

private:
  tmElements_t time_;
};

class WatchyApp {
public:
  virtual bool show(Watchy *watchy, Display *display) = 0;
  virtual bool fetchNetwork(Watchy *watchy) { return true; }

  virtual void reset(Watchy *watchy) {}
  virtual void buttonUp(Watchy *watchy) {}
  virtual void buttonDown(Watchy *watchy) {}
  virtual void buttonSelect(Watchy *watchy) {}
  virtual void buttonBack(Watchy *watchy) {}

  virtual ~WatchyApp() = default;
};

#endif
