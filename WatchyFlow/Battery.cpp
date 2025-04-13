#include "Battery.h"
#include "icons.h"
#include "Watchy.h"

const uint8_t BATTERY_SEGMENT_WIDTH   = 7;
const uint8_t BATTERY_SEGMENT_HEIGHT  = 11;
const uint8_t BATTERY_SEGMENT_SPACING = 9;

void LayoutBattery::size(Display *display, uint16_t targetWidth,
                         uint16_t targetHeight, uint16_t *width,
                         uint16_t *height) {
  *width  = 37;
  *height = 21;
}

void LayoutBattery::draw(Display *display, int16_t x0, int16_t y0,
                         uint16_t targetWidth, uint16_t targetHeight,
                         uint16_t *width, uint16_t *height) {
  *width  = 37;
  *height = 21;

  display->drawBitmap(x0, y0, battery, 37, 21, color_);
  int8_t batteryLevel = 0;
  if (vbat_ > 4.0) {
    batteryLevel = 3;
  } else if (vbat_ > 3.6 && vbat_ <= 4.0) {
    batteryLevel = 2;
  } else if (vbat_ > 3.20 && vbat_ <= 3.6) {
    batteryLevel = 1;
  } else if (vbat_ <= 3.20) {
    batteryLevel = 0;
  }

  for (int8_t batterySegments = 0; batterySegments < batteryLevel;
       batterySegments++) {
    display->fillRect(x0 + 5 + (batterySegments * BATTERY_SEGMENT_SPACING),
                      y0 + 5, BATTERY_SEGMENT_WIDTH, BATTERY_SEGMENT_HEIGHT,
                      color_);
  }
}
