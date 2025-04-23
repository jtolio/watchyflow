#include "Watchy.h"
#include "CalendarFace.h"
#include "MenuApp.h"
#include "settings.h"

RTC_DATA_ATTR menuAppMemory rootMenu;

class AboutApp : public WatchyApp {
public:
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

    display->display(partialRefresh);
    return true;
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
  WatchyApp *menuItems[] = {&about, &netFetch, &calReset};
  String menuNames[]     = {"About", "Trigger Network Fetch",
                            "Trigger Calendar Reset"};
  MenuApp menu(&rootMenu, &app, 3, menuItems, menuNames);
  Watchy::wakeup(&menu, watchSettings);
}

void loop() { Watchy::sleep(); }
