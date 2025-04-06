#include "Watchy.h"
#include "CalendarFace.h"
#include "settings.h"

void setup() {
  CalendarFace app(calSettings);
  Watchy::wakeup(&app, watchSettings);
}

void loop() { Watchy::sleep(); }
