#include "Layout.h"

#define SCREEN_SIZE 200

LayoutBitmap::LayoutBitmap(const uint8_t *bitmap, uint16_t w, uint16_t h, uint16_t color)
  : bitmap_(bitmap), w_(w), h_(h), color_(color) {}

void LayoutBitmap::size(uint16_t availableWidth, uint16_t availableHeight,
                        uint16_t *width, uint16_t *height) {
  *width = w_;
  *height = h_;
}

void LayoutBitmap::draw(int16_t x0, int16_t y0,
                        uint16_t availableWidth, uint16_t availableHeight,
                        uint16_t *width, uint16_t *height) {
  Watchy::Watchy::display.drawBitmap(x0, y0, bitmap_, w_, h_, color_);
  *width = w_;
  *height = h_;
}

LayoutText::LayoutText(String text, const GFXfont *font, uint16_t color)
  : text_(text), font_(font), color_(color), rotation_(0) {}

void LayoutText::size(uint16_t availableWidth, uint16_t availableHeight,
                      uint16_t *width, uint16_t *height) {
  int16_t x1, y1;
  Watchy::Watchy::display.setFont(font_);
  Watchy::Watchy::display.getTextBounds(text_, 0, 0, &x1, &y1, width, height);
  uint16_t swap;
  if (rotation_ == 1 || rotation_ == 3) {
    swap = *width;
    *width = *height;
    *height = swap;
  }
}

void LayoutText::draw(int16_t x0, int16_t y0,
                      uint16_t availableWidth, uint16_t availableHeight,
                      uint16_t *width, uint16_t *height) {
  int16_t x1, y1;
  uint16_t swap;
  Watchy::Watchy::display.setFont(font_);
  Watchy::Watchy::display.getTextBounds(text_, 0, 0, &x1, &y1, width, height);
  Watchy::Watchy::display.setTextColor(color_);

  switch (rotation_) {
    default:
      break;
    case 1:
      swap = *width;
      *width = *height;
      *height = swap;
      swap = x0;
      x0 = y0;
      y0 = SCREEN_SIZE - swap - *width;
      break;
    case 2:
      x0 = SCREEN_SIZE - x0 - *height;
      y0 = SCREEN_SIZE - y0 - *width;
      break;
    case 3:
      swap = *width;
      *width = *height;
      *height = swap;
      swap = y0;
      y0 = x0;
      x0 = SCREEN_SIZE - swap - *height;
      break;
  }

  Watchy::Watchy::display.setRotation(rotation_);
  Watchy::Watchy::display.setCursor(x0 - x1, y0 - y1);
  Watchy::Watchy::display.print(text_);
  Watchy::Watchy::display.setRotation(0);
}

LayoutColumns::LayoutColumns(uint16_t columnCount, LayoutElement *columns[],
                             bool hstretch[])
  : columnCount_(columnCount), columns_(columns), hstretch_(hstretch) {}

void LayoutColumns::size(uint16_t availableWidth, uint16_t availableHeight,
                         uint16_t *width, uint16_t *height) {
  *height = availableHeight;
  *width = 0;
  bool canStretch = false;

  for (uint16_t i = 0; i < columnCount_; i++) {
    if (hstretch_[i]) {
      canStretch = true;
    }
    uint16_t columnWidth, columnHeight;
    columns_[i]->size(0, availableHeight,
                      &columnWidth, &columnHeight);
    if (columnHeight > *height) {
      *height = columnHeight;
    }
    *width += columnWidth;
  }

  if (*width < availableWidth && canStretch) {
    *width = availableWidth;
  }
}

void LayoutColumns::draw(int16_t x0, int16_t y0,
                         uint16_t availableWidth, uint16_t availableHeight,
                         uint16_t *width, uint16_t *height) {
  uint16_t fixedWidth = 0;
  uint16_t splits = 0;

  for (uint16_t i = 0; i < columnCount_; i++) {
    uint16_t subwidth, subheight;
    columns_[i]->size(0, availableHeight,
                      &subwidth, &subheight);
    if (subheight > availableHeight) {
      availableHeight = subheight;
    }
    if (hstretch_[i]) {
      splits++;
      continue;
    }
    fixedWidth += subwidth;
  }

  uint16_t remainingWidth = 0;
  if (availableWidth > fixedWidth) {
    remainingWidth = availableWidth - fixedWidth;
  }

  *width = 0;
  *height = 0;
  for (uint16_t i = 0; i < columnCount_; i++) {
    uint16_t subAvailableWidth = 0;
    if (hstretch_[i]) {
      subAvailableWidth = remainingWidth / splits;
      remainingWidth -= subAvailableWidth;
      splits--;
    }
    uint16_t subwidth, subheight;
    columns_[i]->draw(x0 + *width, y0,
                      subAvailableWidth, availableHeight,
                      &subwidth, &subheight);
    *width += subwidth;
    if (subheight > *height) {
      *height = subheight;
    }
  }
}

LayoutRows::LayoutRows(uint16_t rowCount, LayoutElement *rows[],
                       bool vstretch[])
  : rowCount_(rowCount), rows_(rows), vstretch_(vstretch) {}

void LayoutRows::size(uint16_t availableWidth, uint16_t availableHeight,
                      uint16_t *width, uint16_t *height) {
  *width = availableWidth;
  *height = 0;
  bool canStretch = false;

  for (uint16_t i = 0; i < rowCount_; i++) {
    if (vstretch_[i]) {
      canStretch = true;
    }
    uint16_t rowWidth, rowHeight;
    rows_[i]->size(availableWidth, 0,
                   &rowWidth, &rowHeight);
    if (rowWidth > *width) {
      *width = rowWidth;
    }
    *height += rowHeight;
  }

  if (*height < availableHeight && canStretch) {
    *height = availableHeight;
  }
}

void LayoutRows::draw(int16_t x0, int16_t y0,
                      uint16_t availableWidth, uint16_t availableHeight,
                      uint16_t *width, uint16_t *height) {
  uint16_t fixedHeight = 0;
  uint16_t splits = 0;

  for (uint16_t i = 0; i < rowCount_; i++) {
    uint16_t subwidth, subheight;
    rows_[i]->size(availableWidth, 0,
                   &subwidth, &subheight);
    if (subwidth > availableWidth) {
      availableWidth = subwidth;
    }
    if (vstretch_[i]) {
      splits++;
      continue;
    }
    fixedHeight += subheight;
  }

  uint16_t remainingHeight = 0;
  if (availableHeight > fixedHeight) {
    remainingHeight = availableHeight - fixedHeight;
  }

  *width = 0;
  *height = 0;
  for (uint16_t i = 0; i < rowCount_; i++) {
    uint16_t subAvailableHeight = 0;
    if (vstretch_[i]) {
      subAvailableHeight = remainingHeight / splits;
      remainingHeight -= subAvailableHeight;
      splits--;
    }
    uint16_t subwidth, subheight;
    rows_[i]->draw(x0, y0 + *height,
                   availableWidth, subAvailableHeight,
                   &subwidth, &subheight);
    *height += subheight;
    if (subwidth > *width) {
      *width = subwidth;
    }
  }
}

LayoutPadChild::LayoutPadChild(LayoutElement *child,
                             int16_t padTop, int16_t padRight,
                             int16_t padBottom, int16_t padLeft)
  : child_(child), padTop_(padTop), padRight_(padRight),
    padBottom_(padBottom), padLeft_(padLeft) {}

void LayoutPadChild::size(uint16_t availableWidth, uint16_t availableHeight,
                         uint16_t *width, uint16_t *height) {
  int16_t signedAvailableWidth = (int16_t)availableWidth;
  int16_t signedAvailableHeight = (int16_t)availableHeight;
  signedAvailableWidth -= (padLeft_ + padRight_);
  signedAvailableHeight -= (padTop_ + padBottom_);
  availableWidth = (signedAvailableWidth < 0) ?
      0 : (uint16_t)signedAvailableWidth;
  availableHeight = (signedAvailableHeight < 0) ?
      0 : (uint16_t)signedAvailableHeight;
  child_->size(availableWidth, availableHeight, width, height);
  *width += padLeft_ + padRight_;
  *height += padTop_ + padBottom_;
}

void LayoutPadChild::draw(int16_t x0, int16_t y0,
                         uint16_t availableWidth, uint16_t availableHeight,
                         uint16_t *width, uint16_t *height) {
  int16_t signedAvailableWidth = (int16_t)availableWidth;
  int16_t signedAvailableHeight = (int16_t)availableHeight;
  signedAvailableWidth -= (padLeft_ + padRight_);
  signedAvailableHeight -= (padTop_ + padBottom_);
  availableWidth = (signedAvailableWidth < 0) ?
      0 : (uint16_t)signedAvailableWidth;
  availableHeight = (signedAvailableHeight < 0) ?
      0 : (uint16_t)signedAvailableHeight;
  child_->draw(x0 + padLeft_, y0 + padTop_, availableWidth, availableHeight,
               width, height);
  *width += padLeft_ + padRight_;
  *height += padTop_ + padBottom_;
}
