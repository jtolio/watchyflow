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
    display->setCursor(5, 10);
    display->println("github.com/jtolio/watchy");
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

void setup() {
  CalendarFace app(calSettings);
  AboutApp about;
  TriggerNetworkFetchApp netFetch;
  WatchyApp *menuItems[] = {&about, &netFetch};
  String menuNames[]     = {"About", "Trigger Network Fetch"};
  MenuApp menu(&rootMenu, &app, 2, menuItems, menuNames);
  Watchy::wakeup(&menu, watchSettings);
}

void loop() { Watchy::sleep(); }
