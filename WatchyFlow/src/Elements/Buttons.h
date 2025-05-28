#pragma once

#include "../Layout/Layout.h"

// LayoutButtonLabels will show labels next to each physical button, if this is
// the outermost LayoutElement. You can wrap another LayoutElement with this
// if you want button labels.
class LayoutButtonLabels : public LayoutElement {
public:
  LayoutButtonLabels(Watchy *watchy, String back, String select, String up,
                     String down, const GFXfont *font, uint16_t color,
                     bool verticalLabels, const LayoutElement &child);

  LayoutButtonLabels(const LayoutButtonLabels &copy) : layout_(copy.layout_) {}

  void size(Display *display, uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override {
    layout_->size(display, targetWidth, targetHeight, width, height);
  }

  void draw(Display *display, int16_t x0, int16_t y0, uint16_t targetWidth,
            uint16_t targetHeight, uint16_t *width, uint16_t *height) override {
    layout_->draw(display, x0, y0, targetWidth, targetHeight, width, height);
  }

  LayoutElement::ptr clone() const override {
    // kind of a silly trick, but we don't really need to return a
    // LayoutButtonLabels here, since all it is is a wrapper around the layout
    // we constructed in the constructor. We can just return that layout
    // directly and save some few cycles.
    return layout_;
  }

private:
  LayoutElement::ptr layout_;
};
