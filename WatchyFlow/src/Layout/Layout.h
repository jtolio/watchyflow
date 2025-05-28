#pragma once

#include "../Watchy/Watchy.h"
#include "Arena.h"
#include <memory>
#include <vector>
#include <initializer_list>

// This is the base class for a LayoutElement. You can compose a whole view
// in terms of a hierarchy of LayoutElements, which will then align and orient
// themselves as their parents request.
class LayoutElement {
public:
  typedef std::shared_ptr<LayoutElement> ptr;

public:
  // Every LayoutElement must implement size(). size() is given a targetWidth
  // and a targetHeight. If the parent wants to shrink this element as small
  // as possible, these values might be small, maybe even zero. If the parent
  // wants this element to take up a lot of space, the space it wants it to take
  // will be the targetWidth and targetHeight. It is expected that this
  // LayoutElement will provide the smallest or largest size that it can do but
  // no smaller or larger. The output values are given in *width and *height.
  // size() should always set *width and *height. display is provided to convey
  // the overall display size, but should not be drawn on.
  virtual void size(Display *display, uint16_t targetWidth,
                    uint16_t targetHeight, uint16_t *width,
                    uint16_t *height) = 0;

  // Every LayoutElement must implement draw(). draw() is similar to size() in
  // terms of targetWidth, targetHeight, width, and height. However, draw() also
  // is expected to manifest this element onto display at offset x0, y0.
  virtual void draw(Display *display, int16_t x0, int16_t y0,
                    uint16_t targetWidth, uint16_t targetHeight,
                    uint16_t *width, uint16_t *height) = 0;

  // Every LayoutElement must implement clone, which should return a
  // std::shared_ptr of the concrete descendent type. See some concrete
  // LayoutElements for examples.
  virtual LayoutElement::ptr clone() const = 0;

  // virtual destructor and arena memory management
  virtual ~LayoutElement() = default;
  static void *operator new(size_t size);
  static void *operator new[](size_t size);
  static void operator delete(void *ptr, size_t size) noexcept;
  static void operator delete(void *ptr) noexcept;
  static void operator delete[](void *ptr, size_t size) noexcept;
  static void operator delete[](void *ptr) noexcept;

protected:
  LayoutElement()                                 = default;
  LayoutElement(const LayoutElement &)            = delete;
  LayoutElement &operator=(const LayoutElement &) = delete;
  LayoutElement(LayoutElement &&)                 = delete;
  LayoutElement &operator=(LayoutElement &&)      = delete;
};

// LayoutBitmap takes a pointer to an array of bytes representing a bitmap,
// and the width and height of that bitmap.
class LayoutBitmap : public LayoutElement {
public:
  LayoutBitmap(const uint8_t *bitmap, uint16_t w, uint16_t h, uint16_t color)
      : bitmap_(bitmap), w_(w), h_(h), color_(color) {}
  LayoutBitmap(const LayoutBitmap &copy)
      : bitmap_(copy.bitmap_), w_(copy.w_), h_(copy.h_), color_(copy.color_) {}

  void size(Display *display, uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override {
    *width  = w_;
    *height = h_;
  }

  void draw(Display *display, int16_t x0, int16_t y0, uint16_t targetWidth,
            uint16_t targetHeight, uint16_t *width, uint16_t *height) override {
    display->drawBitmap(x0, y0, bitmap_, w_, h_, color_);
    *width  = w_;
    *height = h_;
  }

  LayoutElement::ptr clone() const override {
    return std::make_shared<LayoutBitmap>(*this);
  }

private:
  const uint8_t *bitmap_;
  uint16_t w_, h_;
  uint16_t color_;
};

// LayoutText takes a string, a font, and a color.
class LayoutText : public LayoutElement {
public:
  LayoutText(String text, const GFXfont *font, uint16_t color)
      : text_(text), font_(font), color_(color) {}
  LayoutText(const LayoutText &copy)
      : text_(copy.text_), font_(copy.font_), color_(copy.color_) {}

  void size(Display *display, uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override;
  void draw(Display *display, int16_t x0, int16_t y0, uint16_t targetWidth,
            uint16_t targetHeight, uint16_t *width, uint16_t *height) override;

  LayoutElement::ptr clone() const override {
    return std::make_shared<LayoutText>(*this);
  }

private:
  String text_;
  const GFXfont *font_;
  uint16_t color_;
};

// LayoutEntry is a container type for lists of cells in LayoutRows or
// LayoutColumns. If stretch is true, it tells the LayoutRows or LayoutColumns
// container to stretch this entry if there is extra space.
class LayoutEntry {
public:
  explicit LayoutEntry(const LayoutElement &elem, bool stretch = false)
      : elem_(elem.clone()), stretch_(stretch) {}

  friend class LayoutColumns;
  friend class LayoutRows;

private:
  LayoutElement::ptr elem_;
  bool stretch_;
};

extern MemArenaAllocator<LayoutEntry> allocatorLayoutEntry;

// LayoutColumns is an element made up of zero or more columns. Column entries
// can either be stretch=true or stretch=false (see LayoutEntry). If
// stretch=false, the column will take up the least amount of space that its
// corresponding column element can. The remaining stretch=true elements will
// be given an equal share of the remaining space.
class LayoutColumns : public LayoutElement {
public:
  LayoutColumns(std::initializer_list<LayoutEntry> elems);
  LayoutColumns(std::vector<LayoutEntry, MemArenaAllocator<LayoutEntry>> elems)
      : elems_(std::move(elems)) {}
  LayoutColumns(const LayoutColumns &copy) : elems_(copy.elems_) {}

  void size(Display *display, uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override;
  void draw(Display *display, int16_t x0, int16_t y0, uint16_t targetWidth,
            uint16_t targetHeight, uint16_t *width, uint16_t *height) override;

  LayoutElement::ptr clone() const override {
    return std::make_shared<LayoutColumns>(*this);
  }

private:
  std::vector<LayoutEntry, MemArenaAllocator<LayoutEntry>> elems_;
};

// LayoutRows is just like LayoutColumns but a vertical collection instead of
// a horizontal collection.
class LayoutRows : public LayoutElement {
public:
  LayoutRows(std::initializer_list<LayoutEntry> elems);
  LayoutRows(std::vector<LayoutEntry, MemArenaAllocator<LayoutEntry>> elems)
      : elems_(std::move(elems)) {}
  LayoutRows(const LayoutRows &copy) : elems_(copy.elems_) {}

  void size(Display *display, uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override;
  void draw(Display *display, int16_t x0, int16_t y0, uint16_t targetWidth,
            uint16_t targetHeight, uint16_t *width, uint16_t *height) override;

  LayoutElement::ptr clone() const override {
    return std::make_shared<LayoutRows>(*this);
  }

private:
  std::vector<LayoutEntry, MemArenaAllocator<LayoutEntry>> elems_;
};

// LayoutFill is an empty element that will take whatever space it is given.
// This can be useful for an empty stretch=true cell in a LayoutRows or
// LayoutColumns.
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

  LayoutElement::ptr clone() const override {
    return std::make_shared<LayoutFill>();
  }
};

// LayoutCenter will take another element and center it, vertically and
// horizontally, within the available space.
class LayoutCenter : public LayoutElement {
public:
  explicit LayoutCenter(const LayoutElement &child) : child_(child.clone()) {}
  LayoutCenter(const LayoutCenter &copy) : child_(copy.child_) {}

  void size(Display *display, uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override;
  void draw(Display *display, int16_t x0, int16_t y0, uint16_t targetWidth,
            uint16_t targetHeight, uint16_t *width, uint16_t *height) override;

  LayoutElement::ptr clone() const override {
    return std::make_shared<LayoutCenter>(*this);
  }

private:
  LayoutElement::ptr child_;
};

// LayoutHCenter will take another element and horizontally center it in the
// available space.
class LayoutHCenter : public LayoutElement {
public:
  explicit LayoutHCenter(const LayoutElement &child) : child_(child.clone()) {}
  LayoutHCenter(const LayoutHCenter &copy) : child_(copy.child_) {}

  void size(Display *display, uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override;
  void draw(Display *display, int16_t x0, int16_t y0, uint16_t targetWidth,
            uint16_t targetHeight, uint16_t *width, uint16_t *height) override;

  LayoutElement::ptr clone() const override {
    return std::make_shared<LayoutHCenter>(*this);
  }

private:
  LayoutElement::ptr child_;
};

// LayoutVCenter will take another element and vertically center it in the
// available space.
class LayoutVCenter : public LayoutElement {
public:
  explicit LayoutVCenter(const LayoutElement &child) : child_(child.clone()) {}
  LayoutVCenter(const LayoutVCenter &copy) : child_(copy.child_) {}

  void size(Display *display, uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override;
  void draw(Display *display, int16_t x0, int16_t y0, uint16_t targetWidth,
            uint16_t targetHeight, uint16_t *width, uint16_t *height) override;

  LayoutElement::ptr clone() const override {
    return std::make_shared<LayoutVCenter>(*this);
  }

private:
  LayoutElement::ptr child_;
};

// LayoutPad will add padding to the top, right, bottom, and left of another
// element.
class LayoutPad : public LayoutElement {
public:
  LayoutPad(const LayoutElement &child, int16_t padTop, int16_t padRight,
            int16_t padBottom, int16_t padLeft);
  LayoutPad(const LayoutPad &copy)
      : child_(copy.child_), padTop_(copy.padTop_), padRight_(copy.padRight_),
        padBottom_(copy.padBottom_), padLeft_(copy.padLeft_) {}

  int16_t padTop() { return padTop_; }
  int16_t padRight() { return padRight_; }
  int16_t padBottom() { return padBottom_; }
  int16_t padLeft() { return padLeft_; }

  void size(Display *display, uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override;

  void draw(Display *display, int16_t x0, int16_t y0, uint16_t targetWidth,
            uint16_t targetHeight, uint16_t *width, uint16_t *height) override;

  LayoutElement::ptr clone() const override {
    return std::make_shared<LayoutPad>(*this);
  }

private:
  LayoutElement::ptr child_;
  int16_t padTop_;
  int16_t padRight_;
  int16_t padBottom_;
  int16_t padLeft_;
};

// LayoutSpacer is a non-drawing element that will only take up a small square
// of the requested size. Useful for adding empty padded cells in a LayoutRows
// or LayoutColumns.
class LayoutSpacer : public LayoutElement {
public:
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

  LayoutElement::ptr clone() const override {
    return std::make_shared<LayoutSpacer>(*this);
  }

private:
  uint16_t size_;
};

// LayoutRotate will rotate the child. 0 means no rotation, 1 means 90 degrees
// clockwise, 2 means 180, 3 means 270. the value is taken % 4.
class LayoutRotate : public LayoutElement {
public:
  LayoutRotate(const LayoutElement &child, uint8_t rotate)
      : child_(child.clone()), rotate_(rotate) {}
  LayoutRotate(const LayoutRotate &copy)
      : child_(copy.child_), rotate_(copy.rotate_) {}

  void size(Display *display, uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override;

  void draw(Display *display, int16_t x0, int16_t y0, uint16_t targetWidth,
            uint16_t targetHeight, uint16_t *width, uint16_t *height) override;

  LayoutElement::ptr clone() const override {
    return std::make_shared<LayoutRotate>(*this);
  }

private:
  LayoutElement::ptr child_;
  uint8_t rotate_;
};

// LayoutBorder will draw a 1 pixel border along each requested side of a
// child element.
class LayoutBorder : public LayoutElement {
public:
  LayoutBorder(const LayoutElement &child, bool top, bool right, bool bottom,
               bool left, uint16_t color);
  LayoutBorder(const LayoutBorder &copy)
      : pad_(copy.pad_), color_(copy.color_) {}

  void size(Display *display, uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override;

  void draw(Display *display, int16_t x0, int16_t y0, uint16_t targetWidth,
            uint16_t targetHeight, uint16_t *width, uint16_t *height) override;

  LayoutElement::ptr clone() const override {
    return std::make_shared<LayoutBorder>(*this);
  }

private:
  LayoutPad pad_;
  uint16_t color_;
};

// LayoutBackground will draw a solid color behind the child.
class LayoutBackground : public LayoutElement {
public:
  LayoutBackground(const LayoutElement &child, uint16_t color)
      : child_(child.clone()), color_(color) {}
  LayoutBackground(const LayoutBackground &copy)
      : child_(copy.child_), color_(copy.color_) {}

  void size(Display *display, uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override;

  void draw(Display *display, int16_t x0, int16_t y0, uint16_t targetWidth,
            uint16_t targetHeight, uint16_t *width, uint16_t *height) override;

  LayoutElement::ptr clone() const override {
    return std::make_shared<LayoutBackground>(*this);
  }

private:
  LayoutElement::ptr child_;
  uint16_t color_;
};

// LayoutOverlay will let you place one child in front of another child.
// This allows you to draw text or notification boxes on top of other things.
class LayoutOverlay : public LayoutElement {
public:
  LayoutOverlay(const LayoutElement &background,
                const LayoutElement &foreground)
      : background_(background.clone()), foreground_(foreground.clone()) {}
  LayoutOverlay(const LayoutOverlay &copy)
      : background_(copy.background_), foreground_(copy.foreground_) {}

  void size(Display *display, uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override;

  void draw(Display *display, int16_t x0, int16_t y0, uint16_t targetWidth,
            uint16_t targetHeight, uint16_t *width, uint16_t *height) override;

  LayoutElement::ptr clone() const override {
    return std::make_shared<LayoutOverlay>(*this);
  }

private:
  LayoutElement::ptr background_;
  LayoutElement::ptr foreground_;
};

// LayoutRightAlign will have the child element fill available space with
// empty space on the left.
class LayoutRightAlign : public LayoutElement {
public:
  explicit LayoutRightAlign(const LayoutElement &child)
      : child_(child.clone()) {}
  LayoutRightAlign(const LayoutRightAlign &copy) : child_(copy.child_) {}

  void size(Display *display, uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override {
    child_->size(display, targetWidth, targetHeight, width, height);
    if (*width < targetWidth) {
      *width = targetWidth;
    }
  }

  void draw(Display *display, int16_t x0, int16_t y0, uint16_t targetWidth,
            uint16_t targetHeight, uint16_t *width, uint16_t *height) override {
    child_->size(display, targetWidth, targetHeight, width, height);
    int16_t adjustment = 0;
    if (*width < targetWidth) {
      adjustment = targetWidth - *width;
    }
    child_->draw(display, x0 + adjustment, y0, targetWidth - adjustment,
                 targetHeight, width, height);
    *width += adjustment;
  }

  LayoutElement::ptr clone() const override {
    return std::make_shared<LayoutRightAlign>(*this);
  }

private:
  LayoutElement::ptr child_;
};

// LayoutBottomAlign will have the child element fill available space with
// empty space on the top.
class LayoutBottomAlign : public LayoutElement {
public:
  explicit LayoutBottomAlign(const LayoutElement &child)
      : child_(child.clone()) {}
  LayoutBottomAlign(const LayoutBottomAlign &copy) : child_(copy.child_) {}

  void size(Display *display, uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override {
    child_->size(display, targetWidth, targetHeight, width, height);
    if (*height < targetHeight) {
      *height = targetHeight;
    }
  }

  void draw(Display *display, int16_t x0, int16_t y0, uint16_t targetWidth,
            uint16_t targetHeight, uint16_t *width, uint16_t *height) override {
    child_->size(display, targetWidth, targetHeight, width, height);
    int16_t adjustment = 0;
    if (*height < targetHeight) {
      adjustment = targetHeight - *height;
    }
    child_->draw(display, x0, y0 + adjustment, targetWidth,
                 targetHeight - adjustment, width, height);
    *height += adjustment;
  }

  LayoutElement::ptr clone() const override {
    return std::make_shared<LayoutBottomAlign>(*this);
  }

private:
  LayoutElement::ptr child_;
};

// LayoutCell is a mutable LayoutElement that holds another child. This can
// be useful if you are constructing large hierarchies of elements, and then
// need to perform some logic to determine some node in the hierarchy tree.
// LayoutCell's notable method is set().
class LayoutCell : public LayoutElement {
public:
  LayoutCell() : child_() {}
  explicit LayoutCell(const LayoutElement &child) : child_(child.clone()) {}
  LayoutCell(const LayoutCell &copy) : child_(copy.child_) {}

  void size(Display *display, uint16_t targetWidth, uint16_t targetHeight,
            uint16_t *width, uint16_t *height) override {
    if (!child_) {
      *width  = 0;
      *height = 0;
      return;
    }
    child_->size(display, targetWidth, targetHeight, width, height);
  }

  void draw(Display *display, int16_t x0, int16_t y0, uint16_t targetWidth,
            uint16_t targetHeight, uint16_t *width, uint16_t *height) override {
    if (!child_) {
      *width  = 0;
      *height = 0;
      return;
    }
    child_->draw(display, x0, y0, targetWidth, targetHeight, width, height);
  }

  void set(const LayoutElement &child) { child_ = child.clone(); }

  LayoutElement::ptr clone() const override {
    return std::make_shared<LayoutCell>(*this);
  }

private:
  LayoutElement::ptr child_;
};
