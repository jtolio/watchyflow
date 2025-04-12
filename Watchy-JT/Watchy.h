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

  time_t defaultTimezoneOffset;
} WatchySettings;

class Watchy {
public:
  static void wakeup(WatchyApp *app, WatchySettings settings);
  static void sleep();

public:
  tmElements_t localtime() { return time_; }
  time_t unixtime() { return toUnixTime(time_); }

  tmElements_t toLocalTime(time_t unix);
  time_t toUnixTime(const tmElements_t &local);

  void vibrate(uint8_t intervalMs = 100, uint8_t length = 20);
  float battVoltage();

  // offset is the offset in seconds that the local time is from UTC.
  // e.g., EST is (-5 * 60 * 60).
  void setTimezoneOffset(time_t offset);

  void triggerNetworkFetch();
  time_t lastSuccessfulNetworkFetch();

protected:
  Watchy(const tmElements_t &currentTime) : time_(currentTime) {}

  static bool syncNTP();

private:
  tmElements_t time_;
};

class WatchyApp {
public:
  virtual bool show(Watchy *watchy, Display *display, bool partialRefresh) = 0;
  virtual bool fetchNetwork(Watchy *watchy) { return true; }

  virtual void reset(Watchy *watchy) {}
  virtual void buttonUp(Watchy *watchy) {}
  virtual void buttonDown(Watchy *watchy) {}
  virtual void buttonSelect(Watchy *watchy) {}
  virtual bool buttonBack(Watchy *watchy) { return true; }

  virtual ~WatchyApp() = default;
};

#endif
