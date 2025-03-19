#include "Layout.h"

LayoutBitmap::LayoutBitmap(const uint8_t *bitmap, uint16_t w, uint16_t h, uint16_t color)
  : bitmap_(bitmap), w_(w), h_(h), color_(color) {}

void LayoutBitmap::size(uint16_t targetWidth, uint16_t targetHeight,
                        uint16_t *width, uint16_t *height) {
  *width = w_;
  *height = h_;
}

void LayoutBitmap::draw(int16_t x0, int16_t y0,
                        uint16_t targetWidth, uint16_t targetHeight,
                        uint16_t *width, uint16_t *height) {
  Watchy::Watchy::display.drawBitmap(x0, y0, bitmap_, w_, h_, color_);
  *width = w_;
  *height = h_;
}

LayoutText::LayoutText(String text, const GFXfont *font, uint16_t color)
  : text_(text), font_(font), color_(color) {}

void LayoutText::size(uint16_t targetWidth, uint16_t targetHeight,
                      uint16_t *width, uint16_t *height) {
  if (text_.length() <= 0) {
    *width = 0;
    *height = 0;
    return;
  }
  int16_t x1, y1;
  Watchy::Watchy::display.setFont(font_);
  Watchy::Watchy::display.getTextBounds(text_, 0, 0, &x1, &y1, width, height);
}

void LayoutText::draw(int16_t x0, int16_t y0,
                      uint16_t targetWidth, uint16_t targetHeight,
                      uint16_t *width, uint16_t *height) {
  if (text_.length() <= 0) {
    *width = 0;
    *height = 0;
    return;
  }
  int16_t x1, y1;
  Watchy::Watchy::display.setFont(font_);
  Watchy::Watchy::display.getTextBounds(text_, 0, 0, &x1, &y1, width, height);
  Watchy::Watchy::display.setTextColor(color_);
  Watchy::Watchy::display.setCursor(x0 - x1, y0 - y1);
  Watchy::Watchy::display.print(text_);
}

void LayoutRotate::size(uint16_t targetWidth, uint16_t targetHeight,
                        uint16_t *width, uint16_t *height) {
  uint16_t swap;
  if (rotate_ == 1 || rotate_ == 3) {
    swap = targetWidth;
    targetWidth = targetHeight;
    targetHeight = swap;
  }
  child_->size(targetWidth, targetHeight, width, height);
  if (rotate_ == 1 || rotate_ == 3) {
    swap = *width;
    *width = *height;
    *height = swap;
  }
}

void LayoutRotate::draw(int16_t x0, int16_t y0,
                        uint16_t targetWidth, uint16_t targetHeight,
                        uint16_t *width, uint16_t *height) {
  uint16_t swap;
  if (rotate_ == 1 || rotate_ == 3) {
    swap = targetWidth;
    targetWidth = targetHeight;
    targetHeight = swap;
  }
  child_->size(targetWidth, targetHeight, width, height);

  switch (rotate_) {
    default:
      break;
    case 1:
      swap = *width;
      *width = *height;
      *height = swap;
      swap = x0;
      x0 = y0;
      y0 = Watchy::Watchy::display.width() - swap - *width;
      break;
    case 2:
      x0 = Watchy::Watchy::display.width() - x0 - *height;
      y0 = Watchy::Watchy::display.height() - y0 - *width;
      break;
    case 3:
      swap = *width;
      *width = *height;
      *height = swap;
      swap = y0;
      y0 = x0;
      x0 = Watchy::Watchy::display.height() - swap - *height;
      break;
  }

  uint8_t currentRotation = Watchy::Watchy::display.getRotation();
  Watchy::Watchy::display.setRotation((currentRotation + rotate_) % 4);
  child_->draw(x0, y0, targetWidth, targetHeight, width, height);
  Watchy::Watchy::display.setRotation(currentRotation);
  if (rotate_ == 1 || rotate_ == 3) {
    swap = *width;
    *width = *height;
    *height = swap;
  }
}

LayoutColumns::LayoutColumns(uint16_t columnCount, LayoutElement *columns[],
                             bool hstretch[])
  : columnCount_(columnCount), columns_(columns), hstretch_(hstretch) {}

void LayoutColumns::size(uint16_t targetWidth, uint16_t targetHeight,
                         uint16_t *width, uint16_t *height) {
  *height = targetHeight;
  *width = 0;
  bool canStretch = false;

  for (uint16_t i = 0; i < columnCount_; i++) {
    if (hstretch_[i]) {
      canStretch = true;
    }
    uint16_t columnWidth, columnHeight;
    columns_[i]->size(0, targetHeight,
                      &columnWidth, &columnHeight);
    if (columnHeight > *height) {
      *height = columnHeight;
    }
    *width += columnWidth;
  }

  if (*width < targetWidth && canStretch) {
    *width = targetWidth;
  }
}

void LayoutColumns::draw(int16_t x0, int16_t y0,
                         uint16_t targetWidth, uint16_t targetHeight,
                         uint16_t *width, uint16_t *height) {
  uint16_t fixedWidth = 0;
  uint16_t splits = 0;

  for (uint16_t i = 0; i < columnCount_; i++) {
    uint16_t subwidth, subheight;
    columns_[i]->size(0, targetHeight,
                      &subwidth, &subheight);
    if (subheight > targetHeight) {
      targetHeight = subheight;
    }
    if (hstretch_[i]) {
      splits++;
      continue;
    }
    fixedWidth += subwidth;
  }

  uint16_t remainingWidth = 0;
  if (targetWidth > fixedWidth) {
    remainingWidth = targetWidth - fixedWidth;
  }

  *width = 0;
  *height = 0;
  for (uint16_t i = 0; i < columnCount_; i++) {
    uint16_t subTargetWidth = 0;
    if (hstretch_[i]) {
      subTargetWidth = remainingWidth / splits;
      remainingWidth -= subTargetWidth;
      splits--;
    }
    uint16_t subwidth, subheight;
    columns_[i]->draw(x0 + *width, y0,
                      subTargetWidth, targetHeight,
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

void LayoutRows::size(uint16_t targetWidth, uint16_t targetHeight,
                      uint16_t *width, uint16_t *height) {
  *width = targetWidth;
  *height = 0;
  bool canStretch = false;

  for (uint16_t i = 0; i < rowCount_; i++) {
    if (vstretch_[i]) {
      canStretch = true;
    }
    uint16_t rowWidth, rowHeight;
    rows_[i]->size(targetWidth, 0,
                   &rowWidth, &rowHeight);
    if (rowWidth > *width) {
      *width = rowWidth;
    }
    *height += rowHeight;
  }

  if (*height < targetHeight && canStretch) {
    *height = targetHeight;
  }
}

void LayoutRows::draw(int16_t x0, int16_t y0,
                      uint16_t targetWidth, uint16_t targetHeight,
                      uint16_t *width, uint16_t *height) {
  uint16_t fixedHeight = 0;
  uint16_t splits = 0;

  for (uint16_t i = 0; i < rowCount_; i++) {
    uint16_t subwidth, subheight;
    rows_[i]->size(targetWidth, 0,
                   &subwidth, &subheight);
    if (subwidth > targetWidth) {
      targetWidth = subwidth;
    }
    if (vstretch_[i]) {
      splits++;
      continue;
    }
    fixedHeight += subheight;
  }

  uint16_t remainingHeight = 0;
  if (targetHeight > fixedHeight) {
    remainingHeight = targetHeight - fixedHeight;
  }

  *width = 0;
  *height = 0;
  for (uint16_t i = 0; i < rowCount_; i++) {
    uint16_t subTargetHeight = 0;
    if (vstretch_[i]) {
      subTargetHeight = remainingHeight / splits;
      remainingHeight -= subTargetHeight;
      splits--;
    }
    uint16_t subwidth, subheight;
    rows_[i]->draw(x0, y0 + *height,
                   targetWidth, subTargetHeight,
                   &subwidth, &subheight);
    *height += subheight;
    if (subwidth > *width) {
      *width = subwidth;
    }
  }
}

void LayoutCenter::size(uint16_t targetWidth, uint16_t targetHeight,
                        uint16_t *width, uint16_t *height) {
  child_->size(targetWidth, targetHeight, width, height);
  if (*width < targetWidth) {
    *width = targetWidth;
  }
  if (*height < targetHeight) {
    *height = targetHeight;
  }
}

void LayoutCenter::draw(int16_t x0, int16_t y0,
                        uint16_t targetWidth, uint16_t targetHeight,
                        uint16_t *width, uint16_t *height) {
  int16_t x0_offset = 0, y0_offset = 0;

  child_->size(targetWidth, targetHeight, width, height);
  if (*width < targetWidth) {
    x0_offset = (targetWidth - *width) / 2;
  }
  if (*height < targetHeight) {
    y0_offset = (targetHeight - *height) / 2;
  }

  child_->draw(x0 + x0_offset, y0 + y0_offset, *width, *height, width, height);

  *width += x0_offset;
  *height += y0_offset;
  if (*width < targetWidth) {
    *width = targetWidth;
  }
  if (*height < targetHeight) {
    *height = targetHeight;
  }
}

LayoutPad::LayoutPad(LayoutElement *child,
                     int16_t padTop, int16_t padRight,
                     int16_t padBottom, int16_t padLeft)
  : child_(child), padTop_(padTop), padRight_(padRight),
    padBottom_(padBottom), padLeft_(padLeft) {}

void LayoutPad::size(uint16_t targetWidth, uint16_t targetHeight,
                     uint16_t *width, uint16_t *height) {
  int16_t signedTargetWidth = (int16_t)targetWidth;
  int16_t signedTargetHeight = (int16_t)targetHeight;
  signedTargetWidth -= (padLeft_ + padRight_);
  signedTargetHeight -= (padTop_ + padBottom_);
  targetWidth = (signedTargetWidth < 0) ?
      0 : (uint16_t)signedTargetWidth;
  targetHeight = (signedTargetHeight < 0) ?
      0 : (uint16_t)signedTargetHeight;
  child_->size(targetWidth, targetHeight, width, height);
  *width += padLeft_ + padRight_;
  *height += padTop_ + padBottom_;
}

void LayoutPad::draw(int16_t x0, int16_t y0,
                     uint16_t targetWidth, uint16_t targetHeight,
                     uint16_t *width, uint16_t *height) {
  int16_t signedTargetWidth = (int16_t)targetWidth;
  int16_t signedTargetHeight = (int16_t)targetHeight;
  signedTargetWidth -= (padLeft_ + padRight_);
  signedTargetHeight -= (padTop_ + padBottom_);
  targetWidth = (signedTargetWidth < 0) ?
      0 : (uint16_t)signedTargetWidth;
  targetHeight = (signedTargetHeight < 0) ?
      0 : (uint16_t)signedTargetHeight;
  child_->draw(x0 + padLeft_, y0 + padTop_, targetWidth, targetHeight,
               width, height);
  *width += padLeft_ + padRight_;
  *height += padTop_ + padBottom_;
}

LayoutBorder::LayoutBorder(LayoutElement *child,
                           bool top, bool right, bool bottom, bool left,
                           uint16_t color)
  : pad_(child, top ? 1 : 0, right ? 1 : 0, bottom ? 1 : 0, left ? 1 : 0),
    color_(color) {}

void LayoutBorder::size(uint16_t targetWidth, uint16_t targetHeight,
                     uint16_t *width, uint16_t *height) {
  pad_.size(targetWidth, targetHeight, width, height);
}

void LayoutBorder::draw(int16_t x0, int16_t y0,
                     uint16_t targetWidth, uint16_t targetHeight,
                     uint16_t *width, uint16_t *height) {
  pad_.draw(x0, y0, targetWidth, targetHeight, width, height);
  if (pad_.padTop() > 0) {
    Watchy::Watchy::display.drawFastHLine(x0, y0, *width, color_);
  }
  if (pad_.padBottom() > 0) {
    Watchy::Watchy::display.drawFastHLine(x0, y0 + *height - 1, *width, color_);
  }
  if (pad_.padLeft() > 0) {
    Watchy::Watchy::display.drawFastVLine(x0, y0, *height, color_);
  }
  if (pad_.padRight() > 0) {
    Watchy::Watchy::display.drawFastVLine(x0 + *width - 1, y0, *height, color_);
  }
}
