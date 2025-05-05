#include "src/Watchy/Watchy.h"
#include "src/Apps/About/About.h"
#include "src/Apps/Tools/Tools.h"
#include "src/Apps/Calendar/CalendarFace.h"
#include "src/Apps/Menu/MenuApp.h"
#include "src/Apps/Stopwatch/Stopwatch.h"
#include "settings.h"

RTC_DATA_ATTR menuAppMemory rootMenu;

void setup() {
  CalendarFace app(calSettings);

  AboutApp about;
  TriggerNetworkFetchApp netFetch;
  TriggerCalendarReset calReset(&app);
  ResetStepCounter resetSteps;

  StopwatchApp stopwatch;

  MenuApp menu(&rootMenu, &app,
               {
                   MenuItem("Stopwatch", &stopwatch),
                   MenuItem("About", &about),
                   MenuItem("Network Fetch", &netFetch),
                   MenuItem("Calendar Reset", &calReset),
                   MenuItem("Reset Steps", &resetSteps),
               });

  Watchy::wakeup(&menu, watchSettings);
}

void loop() {
  AboutApp::presleep();
  Watchy::sleep();
}
