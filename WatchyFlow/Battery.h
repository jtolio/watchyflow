#ifndef BATTERY_H
#define BATTERY_H

#include "Watchy.h"
#include "Layout.h"

class LayoutBattery : public LayoutElement {
public:
  explicit LayoutBattery(Watchy *watchy, uint16_t color)
      : watchy_(watchy), color_(color) {}

  void size(Display *display, uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override;

  void draw(Display *display, int16_t x0, int16_t y0, uint16_t targetWidth,
            uint16_t targetHeight, uint16_t *width, uint16_t *height) override;

private:
  Watchy *watchy_;
  uint16_t color_;
};

#endif
