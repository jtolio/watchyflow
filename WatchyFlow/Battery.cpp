#include "Battery.h"
#include "icons.h"
#include "Watchy.h"

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

  float vbat = float(watchy_->battPercent()) / 100;

  display->drawBitmap(x0, y0, battery, 37, 21, color_);
  display->fillRect(x0 + 5, y0 + 5, int(26 * vbat), 11, color_);
}
