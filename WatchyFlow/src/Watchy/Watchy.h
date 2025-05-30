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

// Watchy is the main type that manages the Watchy device. It is created through
// the static wakeup() call and passed to your app as it manages your app's
// lifecycle. It should be used from the core Arduino main file like so:
//
//     #include "src/Watchy/Watchy.h"
//     #include "settings.h"
//
//     void setup() {
//       MyApp myapp;
//       Watchy::wakeup(&myapp, watchSettings);
//     }
//
//     void loop() {
//       Watchy::sleep();
//     }
//
class Watchy {
public:
  // wakeup() creates a Watchy and calls the appropriate things on your
  // WatchyApp.
  static void wakeup(WatchyApp *app, WatchySettings settings);

  // sleep puts the ESP32 back into deep sleep with appropriate settings to
  // restart.
  static void sleep();

public:
  // the core methods - these return at what time the watch woke up
  // localtime() is in your local timezone.
  tmElements_t localtime() const { return localtime_; }
  // unixtime() is in UTC.
  time_t unixtime() const { return unixtime_; }

  // these methods convert between the time types. they use your current
  // timezone to do so.
  tmElements_t toLocalTime(time_t unix);
  time_t toUnixTime(const tmElements_t &local);

  // offset is the offset in seconds that the local time is from UTC.
  // e.g., EST is (-5 * 60 * 60). EDT is (-4 * 60 * 60).
  void setTimezoneOffset(time_t offset);

  // to deduplicate different apps calling vibrate, queueVibrate will save
  // the longest vibration requested, and perform that when the screen has
  // drawn.
  void queueVibrate(uint8_t intervalMs = 100, uint8_t length = 20);

  // battery information. battPercent is preferred where possible.
  int battPercent();
  float battVoltage();

  // Why did the watchy wake up? It might not be due to the clock.
  WakeupReason wakeupReason() const { return wakeup_; }

  void triggerNetworkFetch();
  time_t lastSuccessfulNetworkFetch();

  uint32_t stepCounter();
  void resetStepCounter();

  uint8_t temperature(); // celsius

  bool accel(AccelData &acc);
  uint8_t direction();

  ButtonConfiguration buttonConfig() const { return settings_.buttonConfig; }

  uint16_t foregroundColor() const;
  uint16_t backgroundColor() const;

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
  void drawNotice(char *msg);

  void updateScreen(WatchyApp *app, bool partialRefresh);

private:
  tmElements_t localtime_;
  time_t unixtime_;
  WakeupReason wakeup_;
  WatchySettings settings_;
  uint8_t vibrateIntervalMs_;
  uint8_t vibrateLength_;
};
