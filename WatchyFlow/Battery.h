#pragma once

#include "Watchy.h"
#include "Layout.h"

class LayoutBattery : public LayoutElement {
public:
  explicit LayoutBattery(Watchy *watchy, uint16_t color)
      : watchy_(watchy), color_(color) {}
  LayoutBattery(const LayoutBattery &copy)
      : watchy_(copy.watchy_), color_(copy.color_) {}

  void size(Display *display, uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override;

  void draw(Display *display, int16_t x0, int16_t y0, uint16_t targetWidth,
            uint16_t targetHeight, uint16_t *width, uint16_t *height) override;

  LayoutElement::ptr clone() const override {
    return std::make_shared<LayoutBattery>(*this);
  }

private:
  Watchy *watchy_;
  uint16_t color_;
};
