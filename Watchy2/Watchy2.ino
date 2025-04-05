#include "Watchy.h"

#include "DSEG7_Classic_Bold_53.h"

class DumbApp : public WatchyApp {
  bool show(Watchy *watchy) {
    auto display     = watchy->display();
    auto currentTime = watchy->time();
    display->setFont(&DSEG7_Classic_Bold_53);
    display->setCursor(5, 53 + 60);
    if (currentTime.Hour < 10) {
      display->print("0");
    }
    display->print(currentTime.Hour);
    display->print(":");
    if (currentTime.Minute < 10) {
      display->print("0");
    }
    display->println(currentTime.Minute);
    return true;
  }
};

void setup() {
  DumbApp app;
  Watchy::wakeup(&app);
}

void loop() {}
