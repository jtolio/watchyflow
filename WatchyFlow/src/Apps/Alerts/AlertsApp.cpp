#include "AlertsApp.h"
#include "../Calendar/Calendar.h"
#include "../../Elements/Buttons.h"
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>

namespace {
const GFXfont *HIGHLIGHT_FONT = &FreeSansBold9pt7b;
const GFXfont *FONT           = &FreeSans9pt7b;

RTC_DATA_ATTR alarmsData alerts_;
RTC_DATA_ATTR bool alertsShown_;
RTC_DATA_ATTR uint8_t highlightedAlert_;
} // namespace

AppState AlertsApp::show(Watchy *watchy, Display *display) {
  alertsShown_ = alerts_.alarmCount > 0;
  if (!alertsShown_) {
    return app_->show(watchy, display);
  }

  const uint16_t color = watchy->foregroundColor();
  display->fillScreen(watchy->backgroundColor());
  display->setTextWrap(false);

  std::vector<LayoutEntry, MemArenaAllocator<LayoutEntry>> alerts(
      allocatorLayoutEntry);
  alerts.reserve(alerts_.alarmCount + 2);

  alerts.push_back(LayoutEntry(LayoutBackground(
      LayoutHCenter(LayoutPad(
          LayoutText("Alerts", FONT, watchy->backgroundColor()), 3, 0, 3, 0)),
      color)));

  uint8_t selected = highlightedAlert_ % alerts_.alarmCount;

  for (int i = 0; i < alerts_.alarmCount; i++) {
    tmElements_t alertTime = watchy->toLocalTime(alerts_.alarms[i].start);
    int displayHour        = ((alertTime.Hour + 11) % 12) + 1;
    String text            = String(displayHour);
    text += (alertTime.Minute < 10) ? ":0" : ":";
    text += String(alertTime.Minute);
    text += ": ";
    text += alerts_.alarms[i].summary;
    alerts.push_back(LayoutEntry(LayoutPad(
        LayoutText(text, i == selected ? HIGHLIGHT_FONT : FONT, color), 3, 0, 3,
        0)));
  }

  alerts.push_back(LayoutEntry(LayoutFill(), true));

  uint16_t w, h;
  LayoutButtonLabels(watchy, "", "Confirm", "Up", "Down", NULL, color, false,
                     LayoutRows(alerts))
      .draw(display, 0, 0, display->width(), display->height(), &w, &h);

  return APP_ACTIVE;
}

void AlertsApp::reset(Watchy *watchy) {
  ::reset(&alerts_);
  alertsShown_      = false;
  highlightedAlert_ = 0;
  app_->reset(watchy);
}

void AlertsApp::buttonUp(Watchy *watchy) {
  if (!alertsShown_) {
    app_->buttonUp(watchy);
    return;
  }
  highlightedAlert_ = (highlightedAlert_ + 1) % alerts_.alarmCount;
}

void AlertsApp::buttonDown(Watchy *watchy) {
  if (!alertsShown_) {
    app_->buttonDown(watchy);
    return;
  }
  highlightedAlert_ =
      (highlightedAlert_ + alerts_.alarmCount - 1) % alerts_.alarmCount;
}

AppState AlertsApp::buttonSelect(Watchy *watchy) {
  if (!alertsShown_) {
    return app_->buttonSelect(watchy);
  }
  if (alerts_.alarmCount <= 0) {
    return APP_EXIT;
  }
  uint8_t selected = highlightedAlert_ % alerts_.alarmCount;

  for (uint8_t i = selected + 1; i < alerts_.alarmCount; i++) {
    alerts_.alarms[i - 1] = alerts_.alarms[i];
  }
  alerts_.alarmCount--;
  if (alerts_.alarmCount == 0) {
    alertsShown_ = false;
    return APP_EXIT;
  }
  return APP_ACTIVE;
}

AppState AlertsApp::buttonBack(Watchy *watchy) {
  if (!alertsShown_) {
    return app_->buttonBack(watchy);
  }
  return APP_ACTIVE;
}

void AlertsApp::tick(Watchy *watchy) {
  app_->tick(watchy);
  if (alerts_.alarmCount > 0) {
    watchy->queueVibrate(200, 10);
  }
}

void AlertsApp::addAlert(String summary, time_t alertTime) {
  addAlarm(&alerts_, summary, alertTime);
}
