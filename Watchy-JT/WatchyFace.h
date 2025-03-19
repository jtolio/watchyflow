#ifndef WATCHYFACE_H
#define WATCHYFACE_H

#include "Watchy.h"
#include "Seven_Segment10pt7b.h"
#include "DSEG7_Classic_Regular_15.h"
#include "DSEG7_Classic_Bold_25.h"
#include "DSEG7_Classic_Regular_39.h"
#include "icons.h"

class WatchyFace : public Watchy {
  using Watchy::Watchy;
public:
  void deviceReset() override;
  void postDraw() override;
  void drawWatchFace() override;
private:
  void parseCalendar(String payload);
};

#endif
