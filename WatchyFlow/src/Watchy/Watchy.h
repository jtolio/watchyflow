#pragma once

#include <TimeLib.h>
#include <GxEPD2_BW.h>
#include "Display.h"

#ifdef ARDUINO_ESP32S3_DEV
#define IS_WATCHY_V3
#else
#define IS_WATCHY_V2
#endif

#include "Settings.h"

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

  void queueVibrate(uint8_t intervalMs = 100, uint8_t length = 20);

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

  ButtonConfiguration buttonConfig() { return settings_.buttonConfig; }

protected:
  Watchy(const tmElements_t &currentTime, WakeupReason wakeup,
         WatchySettings settings)
      : localtime_(currentTime), unixtime_(toUnixTime(currentTime)),
        wakeup_(wakeup), settings_(settings), vibrateIntervalMs_(0),
        vibrateLength_(0) {}

  void reset(const tmElements_t &currentTime, WakeupReason wakeup);
  void queuedVibrate();
  void vibrate(uint8_t intervalMs = 100, uint8_t length = 20);

  static bool syncNTP();
  static void drawNotice(char *msg);

private:
  tmElements_t localtime_;
  time_t unixtime_;
  WakeupReason wakeup_;
  WatchySettings settings_;
  uint8_t vibrateIntervalMs_;
  uint8_t vibrateLength_;
};
