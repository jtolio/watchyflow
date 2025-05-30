#include "Timer.h"
#include "../../Layout/Layout.h"
#include "../../Elements/Buttons.h"
#include "../../Fonts/DSEG7_Classic_Regular_39.h"
#include "../../Fonts/Seven_Segment10pt7b.h"
#include <Fonts/FreeSans9pt7b.h>

namespace {
RTC_DATA_ATTR int16_t minutes_;
RTC_DATA_ATTR int16_t increment0_;
RTC_DATA_ATTR int16_t increment1_;
RTC_DATA_ATTR bool running_;
RTC_DATA_ATTR time_t expiry_;

String twoDigit(time_t val) {
  if (val >= 0 && val < 10) {
    return "0" + String(val);
  }
  return String(val);
}

String durationToString(time_t total) {
  time_t seconds = total % 60;
  total /= 60;
  time_t minutes = total % 60;
  time_t hours   = total / 60;
  if (hours > 9) {
    return "-------";
  }
  return String(hours) + ":" + twoDigit(minutes) + ":" + twoDigit(seconds);
}
} // namespace

void TimerApp::reset(Watchy *watchy) {
  minutes_    = 1;
  expiry_     = 0;
  increment0_ = 0;
  increment1_ = 0;
  running_    = false;
}

void TimerApp::tick(Watchy *watchy) {
  time_t now = watchy->unixtime();
  if (running_ && expiry_ <= now) {
    running_ = false;
    watchy->queueVibrate(100, 10);
  }
}

AppState TimerApp::show(Watchy *watchy, Display *display, bool partialRefresh) {
  const uint16_t FOREGROUND_COLOR = watchy->foregroundColor();
  display->fillScreen(watchy->backgroundColor());
  display->setTextWrap(false);

  time_t now       = watchy->unixtime();
  time_t remaining = 0;
  bool running     = running_ && expiry_ > now;

  if (running) {
    remaining = expiry_ - now;
  } else {
    remaining = minutes_ * 60;
  }

  String remainingStr = durationToString(remaining);

  uint16_t w, h;
  LayoutButtonLabels(
      watchy, "Back", running ? "Stop" : "Start", running ? "" : "More",
      running ? "Refresh" : "Less", NULL, FOREGROUND_COLOR, false,
      LayoutVCenter(LayoutRows({
          LayoutEntry(LayoutHCenter(LayoutText(
              remainingStr, &DSEG7_Classic_Regular_39, FOREGROUND_COLOR))),
          LayoutEntry(LayoutSpacer(5)),
          LayoutEntry(
              LayoutHCenter(LayoutText(running ? "Running..." : "Stopped",
                                       &FreeSans9pt7b, FOREGROUND_COLOR))),
      })))
      .draw(display, 0, 0, display->width(), display->height(), &w, &h);

  display->display(partialRefresh);
  return APP_ACTIVE;
}

void TimerApp::buttonUp(Watchy *watchy) {
  if (running_) {
    return;
  }
  int16_t increment = 1;
  if (increment0_ < 1) {
    increment0_ = 1;
    increment1_ = 1;
  } else {
    increment   = increment0_;
    increment0_ = increment1_;
    increment1_ = increment + increment0_;
  }
  minutes_ += increment;
}

void TimerApp::buttonDown(Watchy *watchy) {
  if (running_) {
    return;
  }
  int16_t increment = -1;
  if (increment0_ > -1) {
    increment0_ = -1;
    increment1_ = -1;
  } else {
    increment   = increment0_;
    increment0_ = increment1_;
    increment1_ = increment + increment0_;
  }
  minutes_ += increment;
  if (minutes_ < 1) {
    minutes_    = 1;
    increment0_ = 0;
    increment1_ = 0;
  }
}

AppState TimerApp::buttonSelect(Watchy *watchy) {
  increment0_ = 0;
  increment1_ = 0;
  if (running_) {
    running_ = false;
    return APP_ACTIVE;
  }
  running_   = true;
  time_t now = watchy->unixtime();
  expiry_    = now + minutes_ * 60;
  return APP_ACTIVE;
}

AppState TimerApp::buttonBack(Watchy *watchy) {
  increment0_ = 0;
  increment1_ = 0;
  return APP_EXIT;
}
