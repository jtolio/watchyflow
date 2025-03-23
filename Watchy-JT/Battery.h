#ifndef BATTERY_H
#define BATTERY_H

#include "Layout.h"

class LayoutBattery : public LayoutElement {
public:
  explicit LayoutBattery(float vbat, uint16_t color)
      : vbat_(vbat), color_(color) {}

  void size(uint16_t targetWidth, uint16_t targetHeight, uint16_t *width,
            uint16_t *height) override;

  void draw(int16_t x0, int16_t y0, uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override;

private:
  float vbat_;
  uint16_t color_;
};

#endif
