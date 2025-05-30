#include "Stopwatch.h"
#include "../../Layout/Layout.h"
#include "../../Elements/Buttons.h"
#include "../../Fonts/DSEG7_Classic_Regular_39.h"
#include "../../Fonts/Seven_Segment10pt7b.h"
#include <Fonts/FreeSans9pt7b.h>

namespace {
RTC_DATA_ATTR time_t start_;
RTC_DATA_ATTR time_t elapsed_;
RTC_DATA_ATTR bool running_;
RTC_DATA_ATTR time_t split_;

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

void StopwatchApp::reset(Watchy *watchy) {
  start_   = 0;
  elapsed_ = 0;
  split_   = 0;
  running_ = false;
}

AppState StopwatchApp::show(Watchy *watchy, Display *display,
                            bool partialRefresh) {
  const uint16_t FOREGROUND_COLOR = watchy->foregroundColor();
  display->fillScreen(watchy->backgroundColor());
  display->setTextWrap(false);

  time_t total = elapsed_;
  if (running_) {
    total += watchy->unixtime() - start_;
  }

  String time  = durationToString(total);
  String split = durationToString(split_);

  uint16_t w, h;
  LayoutButtonLabels(
      watchy, "Back", running_ ? "Stop" : "Start", "Split",
      running_ ? "Refresh" : "Reset", NULL, FOREGROUND_COLOR, false,
      LayoutVCenter(LayoutRows({
          LayoutEntry(LayoutHCenter(
              LayoutText(split, &Seven_Segment10pt7b, FOREGROUND_COLOR))),
          LayoutEntry(LayoutSpacer(5)),
          LayoutEntry(LayoutHCenter(
              LayoutText(time, &DSEG7_Classic_Regular_39, FOREGROUND_COLOR))),
          LayoutEntry(LayoutSpacer(5)),
          LayoutEntry(
              LayoutHCenter(LayoutText(running_ ? "Running..." : "Stopped",
                                       &FreeSans9pt7b, FOREGROUND_COLOR))),
      })))
      .draw(display, 0, 0, display->width(), display->height(), &w, &h);

  display->display(partialRefresh);
  return APP_ACTIVE;
}

void StopwatchApp::buttonUp(Watchy *watchy) {
  time_t total = elapsed_;
  if (running_) {
    total += watchy->unixtime() - start_;
  }
  split_ = total;
}

void StopwatchApp::buttonDown(Watchy *watchy) {
  if (!running_) {
    reset(watchy);
  }
}

AppState StopwatchApp::buttonSelect(Watchy *watchy) {
  time_t total = elapsed_;
  if (running_) {
    total += watchy->unixtime() - start_;
  } else {
    start_ = watchy->unixtime();
  }
  elapsed_ = total;
  running_ = !running_;
  return APP_ACTIVE;
}

AppState StopwatchApp::buttonBack(Watchy *watchy) { return APP_EXIT; }
