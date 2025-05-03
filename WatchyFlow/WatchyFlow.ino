#include "Watchy.h"
#include "CalendarFace.h"
#include "MenuApp.h"
#include "settings.h"
#include "Arena.h"

RTC_DATA_ATTR menuAppMemory rootMenu;
RTC_DATA_ATTR size_t arenaUsed_;
RTC_DATA_ATTR size_t arenaRemaining_;

class AboutApp : public WatchyApp {
public:
  virtual void reset(Watchy *watchy) override {
    arenaUsed_      = 0;
    arenaRemaining_ = 0;
  }

  virtual bool show(Watchy *watchy, Display *display,
                    bool partialRefresh) override {
    display->fillScreen(GxEPD_WHITE);
    display->setTextWrap(true);
    display->setTextColor(GxEPD_BLACK);
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
    return true;
  }
};

class ResetStepCounter : public WatchyApp {
public:
  virtual bool show(Watchy *watchy, Display *display,
                    bool partialRefresh) override {
    watchy->resetStepCounter();
    return false;
  }
};

class TriggerNetworkFetchApp : public WatchyApp {
public:
  virtual bool show(Watchy *watchy, Display *display,
                    bool partialRefresh) override {
    watchy->triggerNetworkFetch();
    return false;
  }
};

class TriggerCalendarReset : public WatchyApp {
public:
  explicit TriggerCalendarReset(CalendarFace *cal) : cal_(cal) {}
  virtual bool show(Watchy *watchy, Display *display,
                    bool partialRefresh) override {
    cal_->forceCacheMiss();
    watchy->triggerNetworkFetch();
    return false;
  }

private:
  CalendarFace *cal_;
};

void setup() {
  CalendarFace app(calSettings);
  AboutApp about;
  TriggerNetworkFetchApp netFetch;
  TriggerCalendarReset calReset(&app);
  ResetStepCounter resetSteps;
  MenuApp menu(&rootMenu, &app,
               {
                   MenuItem("About", &about),
                   MenuItem("Network Fetch", &netFetch),
                   MenuItem("Calendar Reset", &calReset),
                   MenuItem("Reset Steps", &resetSteps),
               });
  Watchy::wakeup(&menu, watchSettings);
}

void loop() {
  if (globalArena.used() > arenaUsed_) {
    arenaUsed_      = globalArena.used();
    arenaRemaining_ = globalArena.remaining();
  }
  Watchy::sleep();
}
