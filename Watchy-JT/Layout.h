#ifndef LAYOUT_H
#define LAYOUT_H

#include "Watchy.h"

class LayoutElement {
public:
  virtual void size(uint16_t targetWidth, uint16_t targetHeight,
                    uint16_t *width, uint16_t *height) = 0;
  virtual void draw(int16_t x0, int16_t y0, uint16_t targetWidth,
                    uint16_t targetHeight, uint16_t *width,
                    uint16_t *height)                  = 0;
  virtual ~LayoutElement()                             = default;
};

class LayoutBitmap : public LayoutElement {
public:
  LayoutBitmap(const uint8_t *bitmap, uint16_t w, uint16_t h, uint16_t color);

  void size(uint16_t targetWidth, uint16_t targetHeight, uint16_t *width,
            uint16_t *height) override;
  void draw(int16_t x0, int16_t y0, uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override;

private:
  const uint8_t *bitmap_;
  uint16_t w_, h_;
  uint16_t color_;
};

class LayoutText : public LayoutElement {
public:
  LayoutText(String text, const GFXfont *font, uint16_t color);

  void size(uint16_t targetWidth, uint16_t targetHeight, uint16_t *width,
            uint16_t *height) override;
  void draw(int16_t x0, int16_t y0, uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override;

private:
  String text_;
  const GFXfont *font_;
  uint16_t color_;
};

class LayoutColumns : public LayoutElement {
public:
  LayoutColumns(uint16_t columnCount, LayoutElement *columns[],
                bool hstretch[]);

  void size(uint16_t targetWidth, uint16_t targetHeight, uint16_t *width,
            uint16_t *height) override;
  void draw(int16_t x0, int16_t y0, uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override;

private:
  uint16_t columnCount_;
  LayoutElement **columns_;
  bool *hstretch_;
};

class LayoutRows : public LayoutElement {
public:
  LayoutRows(uint16_t rowCount, LayoutElement *rows[], bool vstretch[]);

  void size(uint16_t targetWidth, uint16_t targetHeight, uint16_t *width,
            uint16_t *height) override;
  void draw(int16_t x0, int16_t y0, uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override;

private:
  uint16_t rowCount_;
  LayoutElement **rows_;
  bool *vstretch_;
};

class LayoutFill : public LayoutElement {
public:
  void size(uint16_t targetWidth, uint16_t targetHeight, uint16_t *width,
            uint16_t *height) override {
    *width  = targetWidth;
    *height = targetHeight;
  }

  void draw(int16_t x0, int16_t y0, uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override {
    *width  = targetWidth;
    *height = targetHeight;
  }
};

class LayoutCenter : public LayoutElement {
public:
  explicit LayoutCenter(LayoutElement *child) : child_(child) {}

  void size(uint16_t targetWidth, uint16_t targetHeight, uint16_t *width,
            uint16_t *height) override;
  void draw(int16_t x0, int16_t y0, uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override;

private:
  LayoutElement *child_;
};

class LayoutPad : public LayoutElement {
public:
  LayoutPad(LayoutElement *child, int16_t padTop, int16_t padRight,
            int16_t padBottom, int16_t padLeft);

  int16_t padTop() { return padTop_; }
  int16_t padRight() { return padRight_; }
  int16_t padBottom() { return padBottom_; }
  int16_t padLeft() { return padLeft_; }

  void size(uint16_t targetWidth, uint16_t targetHeight, uint16_t *width,
            uint16_t *height) override;

  void draw(int16_t x0, int16_t y0, uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override;

private:
  LayoutElement *child_;
  int16_t padTop_;
  int16_t padRight_;
  int16_t padBottom_;
  int16_t padLeft_;
};

class LayoutSpacer : public LayoutElement {
public:
  explicit LayoutSpacer(uint16_t spacerSize) : size_(spacerSize) {}

  void size(uint16_t targetWidth, uint16_t targetHeight, uint16_t *width,
            uint16_t *height) override {
    *width  = size_;
    *height = size_;
  }

  void draw(int16_t x0, int16_t y0, uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override {
    *width  = size_;
    *height = size_;
  }

private:
  uint16_t size_;
};

class LayoutRotate : public LayoutElement {
public:
  LayoutRotate(LayoutElement *child, uint8_t rotate)
      : child_(child), rotate_(rotate) {}

  void size(uint16_t targetWidth, uint16_t targetHeight, uint16_t *width,
            uint16_t *height) override;

  void draw(int16_t x0, int16_t y0, uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override;

private:
  LayoutElement *child_;
  uint8_t rotate_;
};

class LayoutBorder : public LayoutElement {
public:
  LayoutBorder(LayoutElement *child, bool top, bool right, bool bottom,
               bool left, uint16_t color);

  void size(uint16_t targetWidth, uint16_t targetHeight, uint16_t *width,
            uint16_t *height) override;

  void draw(int16_t x0, int16_t y0, uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override;

private:
  LayoutPad pad_;
  uint16_t color_;
};

#endif
