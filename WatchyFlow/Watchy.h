#ifndef WATCHY_H
#define WATCHY_H

#include <TimeLib.h>
#include <GxEPD2_BW.h>
#include "Display.h"

class WatchyApp;

typedef GxEPD2_BW<WatchyDisplay, WatchyDisplay::HEIGHT> Display;

typedef struct AccelData {
  int16_t x;
  int16_t y;
  int16_t z;
} AccelData;

typedef enum WakeupReason {
  WAKEUP_RESET  = 0,
  WAKEUP_CLOCK  = 1,
  WAKEUP_BUTTON = 2,
  WAKEUP_USB    = 3
} WakeupReason;

typedef struct WiFiConfig {
  String SSID;
  String Pass;
} WiFiConfig;

typedef struct WatchySettings {
  // number of seconds between network fetch attempts
  int networkFetchIntervalSeconds;
  // number of failing network fetch tries before giving up until the next
  // interval.
  int networkFetchTries;

  WiFiConfig *wifiNetworks;
  int wifiNetworkCount;

  time_t defaultTimezoneOffset;

  bool flipButtonSides;
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

  WakeupReason wakeupReason() { return wakeup_; }

  void vibrate(uint8_t intervalMs = 100, uint8_t length = 20);
  float battVoltage();

  // offset is the offset in seconds that the local time is from UTC.
  // e.g., EST is (-5 * 60 * 60).
  void setTimezoneOffset(time_t offset);

  void triggerNetworkFetch();
  time_t lastSuccessfulNetworkFetch();

  uint32_t stepCounter();
  void resetStepCounter();

  uint8_t temperature(); // celsius

  bool accel(AccelData &acc);
  uint8_t direction();

protected:
  Watchy(const tmElements_t &currentTime, WakeupReason wakeup)
      : time_(currentTime), wakeup_(wakeup) {}

  static bool syncNTP();
  static void drawLoading();

private:
  tmElements_t time_;
  WakeupReason wakeup_;
};

class WatchyApp {
public:
  virtual bool show(Watchy *watchy, Display *display, bool partialRefresh) = 0;
  virtual bool fetchNetwork(Watchy *watchy) { return true; }

  virtual void reset(Watchy *watchy) {}
  virtual void buttonUp(Watchy *watchy) {}
  virtual void buttonDown(Watchy *watchy) {}
  virtual bool buttonSelect(Watchy *watchy) { return false; }
  virtual bool buttonBack(Watchy *watchy) { return true; }

  virtual ~WatchyApp() = default;
};

#endif
