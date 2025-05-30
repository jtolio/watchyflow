#include "About.h"
#include "../../Layout/Arena.h"

namespace {
RTC_DATA_ATTR size_t arenaUsed_;
RTC_DATA_ATTR size_t arenaRemaining_;
} // namespace

void AboutApp::reset(Watchy *watchy) {
  arenaUsed_      = 0;
  arenaRemaining_ = 0;
}

AppState AboutApp::show(Watchy *watchy, Display *display, bool partialRefresh) {
  display->fillScreen(watchy->backgroundColor());
  display->setTextWrap(true);
  display->setTextColor(watchy->foregroundColor());
  display->setCursor(0, 0);
  display->println("github.com/jtolio/watchyflow");

  display->print("batt:       ");
  display->print(watchy->battVoltage());
  display->println(" V");

  display->print("time:       ");
  display->println(watchy->unixtime());

  display->print("last fetch: ");
  display->println(watchy->lastSuccessfulNetworkFetch());

  display->print("wakeup:     ");
  display->println(watchy->wakeupReason());

  display->print("steps:      ");
  display->println(watchy->stepCounter());

  display->print("temp:       ");
  uint8_t temp = watchy->temperature();
  display->print(temp);
  display->print("C, ");
  display->print(temp * 9 / 5 + 32);
  display->println("F");

  display->print("arena used: ");
  display->println(arenaUsed_);
  display->print("remaining:  ");
  display->println(arenaRemaining_);

  display->print("direction:  ");
  display->println(watchy->direction());

  display->println("");

  AccelData accel;
  if (watchy->accel(accel)) {
    display->print(accel.x);
    display->print(", ");
    display->print(accel.y);
    display->print(", ");
    display->println(accel.z);
  }

  display->display(partialRefresh);
  return APP_ACTIVE;
}

void AboutApp::presleep() {
  if (globalArena.used() > arenaUsed_) {
    arenaUsed_      = globalArena.used();
    arenaRemaining_ = globalArena.remaining();
  }
}
