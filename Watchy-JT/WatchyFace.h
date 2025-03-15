#ifndef WATCHYFACE_H
#define WATCHYFACE_H

#include "Watchy.h"
#include "Seven_Segment10pt7b.h"
#include "DSEG7_Classic_Regular_15.h"
#include "DSEG7_Classic_Bold_25.h"
#include "DSEG7_Classic_Regular_39.h"
#include "icons.h"

class WatchyFace : public Watchy{
    using Watchy::Watchy;
    public:
        void drawWatchFace();
    private:
        void drawTime(int16_t x0, int16_t y0);
        uint16_t drawDate(int16_t x0, int16_t y0);
        void drawBattery(int16_t x1, int16_t y0);
        void drawWeather();
        void drawCal(int16_t x0, int16_t y0);
};

#endif
