#include "src/Watchy/Watchy.h"
#include "src/Apps/About/About.h"
#include "src/Apps/Tools/Tools.h"
#include "src/Apps/Calendar/CalendarFace.h"
#include "src/Apps/Menu/MenuApp.h"
#include "src/Apps/AltApp/AltApp.h"
#include "src/Apps/Stopwatch/Stopwatch.h"
#include "src/Apps/Timer/Timer.h"
#include "settings.h"

RTC_DATA_ATTR menuAppMemory rootMenuMem;
RTC_DATA_ATTR menuAppMemory toolMenuMem;
RTC_DATA_ATTR altAppMemory rootMem;

void setup() {
  CalendarFace calApp(calSettings);

  AboutApp about;
  TriggerNetworkFetchApp netFetch;
  TriggerCalendarReset calReset(&calApp);
  ResetStepCounter resetSteps;

  StopwatchApp stopwatch;
  TimerApp timer;

  MenuApp toolMenu(&toolMenuMem, "Tools",
                   {
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

  AltApp root(&rootMem, &calApp, &rootMenu);
  Watchy::wakeup(&root, watchSettings);
}

void loop() {
  AboutApp::presleep();
  Watchy::sleep();
}
