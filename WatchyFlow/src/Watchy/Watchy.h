#pragma once

#include <TimeLib.h>
#include <GxEPD2_BW.h>
#include "Display.h"

#ifdef ARDUINO_ESP32S3_DEV
#define IS_WATCHY_V3
#else
#define IS_WATCHY_V2
#endif

class WatchyApp;

typedef GxEPD2_BW<WatchyDisplay, WatchyDisplay::HEIGHT> Display;

typedef struct AccelData {
  int16_t x;
  int16_t y;
  int16_t z;
} AccelData;

typedef enum WakeupReason {
  WAKEUP_RESET    = 0,
  WAKEUP_CLOCK    = 1,
  WAKEUP_BUTTON   = 2,
  WAKEUP_USB      = 3,
  WAKEUP_NETFETCH = 4,
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

  float fullVoltage;
  float emptyVoltage;
} WatchySettings;

class Watchy {
public:
  static void wakeup(WatchyApp *app, WatchySettings settings);
  static void sleep();

public:
  tmElements_t localtime() { return localtime_; }
  time_t unixtime() { return unixtime_; }

  tmElements_t toLocalTime(time_t unix);
  time_t toUnixTime(const tmElements_t &local);

  WakeupReason wakeupReason() { return wakeup_; }

  void vibrate(uint8_t intervalMs = 100, uint8_t length = 20);

  float battVoltage();
  int battPercent();

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
  Watchy(const tmElements_t &currentTime, WakeupReason wakeup,
         WatchySettings settings)
      : localtime_(currentTime), unixtime_(toUnixTime(currentTime)),
        wakeup_(wakeup), settings_(settings) {}

  void reset(const tmElements_t &currentTime, WakeupReason wakeup);

  static bool syncNTP();
  static void drawNotice(char *msg);

private:
  tmElements_t localtime_;
  time_t unixtime_;
  WakeupReason wakeup_;
  WatchySettings settings_;
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
