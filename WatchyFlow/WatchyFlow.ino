#include "src/Watchy/Watchy.h"
#include "src/Apps/About/About.h"
#include "src/Apps/Tools/Tools.h"
#include "src/Apps/Calendar/CalendarApp.h"
#include "src/Apps/Menu/MenuApp.h"
#include "src/Apps/HomeApp/HomeApp.h"
#include "src/Apps/Stopwatch/Stopwatch.h"
#include "src/Apps/Timer/Timer.h"
#include "src/Apps/Alerts/AlertsApp.h"
#include "settings.h"

RTC_DATA_ATTR menuAppMemory rootMenuMem;
RTC_DATA_ATTR menuAppMemory toolMenuMem;
RTC_DATA_ATTR menuAppMemory locationMenuMem;
RTC_DATA_ATTR homeAppMemory rootMem;

void setup() {
  AlertsApp alerts;
  CalendarApp calApp(calSettings, &alerts);

  AboutApp about;
  TriggerNetworkFetchApp netFetch;
  TriggerCalendarResetApp calReset(&calApp);
  ResetStepCounterApp resetSteps;

  StopwatchApp stopwatch;
  TimerApp timer(&alerts);

  SetCalendarLocationApp locationApps[calSettings.locationCount];
  std::vector<MenuItem, MemArenaAllocator<MenuItem>> locationSetters(
      allocatorMenuItem);
  for (int i = 0; i < calSettings.locationCount; i++) {
    locationApps[i] = SetCalendarLocationApp(&calApp, i);
    locationSetters.push_back(
        MenuItem(calSettings.locations[i].name, &(locationApps[i])));
  }

  MenuApp locationMenu(&locationMenuMem, "Location", locationSetters);

  MenuApp toolMenu(&toolMenuMem, "Tools",
                   {
                       MenuItem("Set Location", &locationMenu),
                       MenuItem("Network Fetch", &netFetch),
                       MenuItem("Calendar Reset", &calReset),
                       MenuItem("Reset Steps", &resetSteps),
                   });

  MenuApp rootMenu(&rootMenuMem, "Menu",
                   {
                       MenuItem("Timer", &timer),
                       MenuItem("Stopwatch", &stopwatch),
                       MenuItem("Tools", &toolMenu),
                       MenuItem("About", &about),
                   });

  HomeApp root(&rootMem, &calApp, &rootMenu);
  alerts.setApp(&root);
  Watchy::wakeup(&alerts, watchSettings);
}

void loop() {
  AboutApp::presleep();
  Watchy::sleep();
}
