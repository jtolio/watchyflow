#pragma once

#include "../Layout/Layout.h"

class LayoutWeatherIcon : public LayoutElement {
public:
  explicit LayoutWeatherIcon(bool upToDate, int16_t conditionCode,
                             uint16_t color)
      : upToDate_(upToDate), weather_(conditionCode), color_(color) {}
  LayoutWeatherIcon(const LayoutWeatherIcon &copy)
      : upToDate_(copy.upToDate_), weather_(copy.weather_),
        color_(copy.color_) {}

  void size(Display *display, uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override;

  void draw(Display *display, int16_t x0, int16_t y0, uint16_t targetWidth,
            uint16_t targetHeight, uint16_t *width, uint16_t *height) override;

  LayoutElement::ptr clone() const override {
    return std::make_shared<LayoutWeatherIcon>(*this);
  }

private:
  bool upToDate_;
  int16_t weather_;
  uint16_t color_;
};
