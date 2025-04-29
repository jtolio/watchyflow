#ifndef LAYOUT_H
#define LAYOUT_H

#include "Watchy.h"

class LayoutElement {
public:
  virtual void size(Display *display, uint16_t targetWidth,
                    uint16_t targetHeight, uint16_t *width,
                    uint16_t *height)                  = 0;
  virtual void draw(Display *display, int16_t x0, int16_t y0,
                    uint16_t targetWidth, uint16_t targetHeight,
                    uint16_t *width, uint16_t *height) = 0;
  virtual ~LayoutElement()                             = default;
};

class LayoutBitmap : public LayoutElement {
public:
  LayoutBitmap() : bitmap_(NULL), w_(0), h_(0), color_(0) {}
  LayoutBitmap(const LayoutBitmap &copy)
      : bitmap_(copy.bitmap_), w_(copy.w_), h_(copy.h_), color_(copy.color_) {}

  LayoutBitmap(const uint8_t *bitmap, uint16_t w, uint16_t h, uint16_t color);

  void size(Display *display, uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override;
  void draw(Display *display, int16_t x0, int16_t y0, uint16_t targetWidth,
            uint16_t targetHeight, uint16_t *width, uint16_t *height) override;

private:
  const uint8_t *bitmap_;
  uint16_t w_, h_;
  uint16_t color_;
};

class LayoutText : public LayoutElement {
public:
  LayoutText() : text_(""), font_(NULL), color_(0) {}
  LayoutText(const LayoutText &copy)
      : text_(copy.text_), font_(copy.font_), color_(copy.color_) {}

  LayoutText(String text, const GFXfont *font, uint16_t color);

  void size(Display *display, uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override;
  void draw(Display *display, int16_t x0, int16_t y0, uint16_t targetWidth,
            uint16_t targetHeight, uint16_t *width, uint16_t *height) override;

private:
  String text_;
  const GFXfont *font_;
  uint16_t color_;
};

class LayoutColumns : public LayoutElement {
public:
  LayoutColumns() : columnCount_(0), columns_(NULL), hstretch_(NULL) {}
  LayoutColumns(const LayoutColumns &copy)
      : columnCount_(copy.columnCount_), columns_(copy.columns_),
        hstretch_(copy.hstretch_) {}

  LayoutColumns(uint16_t columnCount, LayoutElement *columns[],
                bool hstretch[]);

  void size(Display *display, uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override;
  void draw(Display *display, int16_t x0, int16_t y0, uint16_t targetWidth,
            uint16_t targetHeight, uint16_t *width, uint16_t *height) override;

private:
  uint16_t columnCount_;
  LayoutElement **columns_;
  bool *hstretch_;
};

class LayoutRows : public LayoutElement {
public:
  LayoutRows() : rowCount_(0), rows_(NULL), vstretch_(NULL) {}
  LayoutRows(const LayoutRows &copy)
      : rowCount_(copy.rowCount_), rows_(copy.rows_),
        vstretch_(copy.vstretch_) {}

  LayoutRows(uint16_t rowCount, LayoutElement *rows[], bool vstretch[]);

  void size(Display *display, uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override;
  void draw(Display *display, int16_t x0, int16_t y0, uint16_t targetWidth,
            uint16_t targetHeight, uint16_t *width, uint16_t *height) override;

private:
  uint16_t rowCount_;
  LayoutElement **rows_;
  bool *vstretch_;
};

class LayoutFill : public LayoutElement {
public:
  void size(Display *display, uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override {
    *width  = targetWidth;
    *height = targetHeight;
  }

  void draw(Display *display, int16_t x0, int16_t y0, uint16_t targetWidth,
            uint16_t targetHeight, uint16_t *width, uint16_t *height) override {
    *width  = targetWidth;
    *height = targetHeight;
  }
};

class LayoutCenter : public LayoutElement {
public:
  LayoutCenter() : child_(NULL) {}
  LayoutCenter(const LayoutCenter &copy) : child_(copy.child_) {}

  explicit LayoutCenter(LayoutElement *child) : child_(child) {}

  void size(Display *display, uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override;
  void draw(Display *display, int16_t x0, int16_t y0, uint16_t targetWidth,
            uint16_t targetHeight, uint16_t *width, uint16_t *height) override;

private:
  LayoutElement *child_;
};

class LayoutHCenter : public LayoutElement {
public:
  LayoutHCenter() : child_(NULL) {}
  LayoutHCenter(const LayoutHCenter &copy) : child_(copy.child_) {}

  explicit LayoutHCenter(LayoutElement *child) : child_(child) {}

  void size(Display *display, uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override;
  void draw(Display *display, int16_t x0, int16_t y0, uint16_t targetWidth,
            uint16_t targetHeight, uint16_t *width, uint16_t *height) override;

private:
  LayoutElement *child_;
};

class LayoutPad : public LayoutElement {
public:
  LayoutPad()
      : child_(NULL), padTop_(0), padRight_(0), padBottom_(0), padLeft_(0) {}
  LayoutPad(const LayoutPad &copy)
      : child_(copy.child_), padTop_(copy.padTop_), padRight_(copy.padRight_),
        padBottom_(copy.padBottom_), padLeft_(copy.padLeft_) {}

  LayoutPad(LayoutElement *child, int16_t padTop, int16_t padRight,
            int16_t padBottom, int16_t padLeft);

  int16_t padTop() { return padTop_; }
  int16_t padRight() { return padRight_; }
  int16_t padBottom() { return padBottom_; }
  int16_t padLeft() { return padLeft_; }

  void size(Display *display, uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override;

  void draw(Display *display, int16_t x0, int16_t y0, uint16_t targetWidth,
            uint16_t targetHeight, uint16_t *width, uint16_t *height) override;

private:
  LayoutElement *child_;
  int16_t padTop_;
  int16_t padRight_;
  int16_t padBottom_;
  int16_t padLeft_;
};

class LayoutSpacer : public LayoutElement {
public:
  LayoutSpacer() : size_(0) {}
  LayoutSpacer(const LayoutSpacer &copy) : size_(copy.size_) {}

  explicit LayoutSpacer(uint16_t spacerSize) : size_(spacerSize) {}

  void size(Display *display, uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override {
    *width  = size_;
    *height = size_;
  }

  void draw(Display *display, int16_t x0, int16_t y0, uint16_t targetWidth,
            uint16_t targetHeight, uint16_t *width, uint16_t *height) override {
    *width  = size_;
    *height = size_;
  }

private:
  uint16_t size_;
};

class LayoutRotate : public LayoutElement {
public:
  LayoutRotate() : child_(NULL), rotate_(0) {}
  LayoutRotate(const LayoutRotate &copy)
      : child_(copy.child_), rotate_(copy.rotate_) {}

  LayoutRotate(LayoutElement *child, uint8_t rotate)
      : child_(child), rotate_(rotate) {}

  void size(Display *display, uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override;

  void draw(Display *display, int16_t x0, int16_t y0, uint16_t targetWidth,
            uint16_t targetHeight, uint16_t *width, uint16_t *height) override;

private:
  LayoutElement *child_;
  uint8_t rotate_;
};

class LayoutBorder : public LayoutElement {
public:
  LayoutBorder() : pad_(), color_(0) {}
  LayoutBorder(const LayoutBorder &copy)
      : pad_(copy.pad_), color_(copy.color_) {}

  LayoutBorder(LayoutElement *child, bool top, bool right, bool bottom,
               bool left, uint16_t color);

  void size(Display *display, uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override;

  void draw(Display *display, int16_t x0, int16_t y0, uint16_t targetWidth,
            uint16_t targetHeight, uint16_t *width, uint16_t *height) override;

private:
  LayoutPad pad_;
  uint16_t color_;
};

class LayoutBackground : public LayoutElement {
public:
  LayoutBackground() : child_(NULL), color_(0) {}
  LayoutBackground(const LayoutBackground &copy)
      : child_(copy.child_), color_(copy.color_) {}

  LayoutBackground(LayoutElement *child, uint16_t color)
      : child_(child), color_(color) {}

  void size(Display *display, uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override;

  void draw(Display *display, int16_t x0, int16_t y0, uint16_t targetWidth,
            uint16_t targetHeight, uint16_t *width, uint16_t *height) override;

private:
  LayoutElement *child_;
  uint16_t color_;
};

class LayoutOverlay : public LayoutElement {
public:
  LayoutOverlay() : background_(NULL), foreground_(NULL) {}
  LayoutOverlay(const LayoutOverlay &copy)
      : background_(copy.background_), foreground_(copy.foreground_) {}

  LayoutOverlay(LayoutElement *background, LayoutElement *foreground)
      : background_(background), foreground_(foreground) {}

  void size(Display *display, uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override;

  void draw(Display *display, int16_t x0, int16_t y0, uint16_t targetWidth,
            uint16_t targetHeight, uint16_t *width, uint16_t *height) override;

private:
  LayoutElement *background_;
  LayoutElement *foreground_;
};

#endif
