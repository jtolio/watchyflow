#include "MenuApp.h"
#include "Layout.h"
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>

#define DARKMODE false

const GFXfont *TITLE_FONT = &FreeSansBold9pt7b;
const GFXfont *FONT       = &FreeSans9pt7b;

const uint16_t FOREGROUND_COLOR = DARKMODE ? GxEPD_WHITE : GxEPD_BLACK;
const uint16_t BACKGROUND_COLOR = DARKMODE ? GxEPD_BLACK : GxEPD_WHITE;

bool MenuApp::show(Watchy *watchy, Display *display, bool partialRefresh) {
  if (fullDrawNeeded_) {
    partialRefresh = false;
  }
  fullDrawNeeded_ = false;

  uint16_t state = memory_->state % (itemCount_ + 2);
  if (state > 1) {
    if (items_[state - 2]->show(watchy, display, partialRefresh)) {
      return true;
    }
    memory_->state = state = 1;
    partialRefresh         = false;
  }
  if (state == 1) {
    if (showMenu(watchy, display, partialRefresh)) {
      return true;
    }
    memory_->state = state = 0;
    partialRefresh         = false;
  }
  return main_->show(watchy, display, partialRefresh);
}

bool MenuApp::fetchNetwork(Watchy *watchy) {
  bool success = main_->fetchNetwork(watchy);
  for (uint16_t i = 0; i < itemCount_; i++) {
    if (!items_[i]->fetchNetwork(watchy)) {
      success = false;
    }
  }
  return success;
}

void MenuApp::reset(Watchy *watchy) {
  memory_->state    = 0;
  memory_->selected = 0;
  main_->reset(watchy);
  for (uint16_t i = 0; i < itemCount_; i++) {
    items_[i]->reset(watchy);
  }
}

void MenuApp::buttonUp(Watchy *watchy) {
  uint16_t state = memory_->state % (itemCount_ + 2);
  switch (state) {
  default:
    items_[state - 2]->buttonUp(watchy);
    return;
  case 1:
    memory_->selected = (memory_->selected + itemCount_ - 1) % itemCount_;
    return;
  case 0:
    main_->buttonUp(watchy);
    return;
  }
}

void MenuApp::buttonDown(Watchy *watchy) {
  uint16_t state = memory_->state % (itemCount_ + 2);
  switch (state) {
  default:
    items_[state - 2]->buttonDown(watchy);
    return;
  case 1:
    memory_->selected = (memory_->selected + 1) % itemCount_;
    return;
  case 0:
    main_->buttonDown(watchy);
    return;
  }
}

void MenuApp::buttonSelect(Watchy *watchy) {
  uint16_t state    = memory_->state % (itemCount_ + 2);
  uint16_t selected = memory_->selected;
  switch (state) {
  default:
    items_[state - 2]->buttonSelect(watchy);
    return;
  case 1:
    selected        = selected % itemCount_;
    memory_->state  = selected + 2;
    fullDrawNeeded_ = true;
    return;
  case 0:
    memory_->state  = 1;
    fullDrawNeeded_ = true;
    return;
  }
}

bool MenuApp::buttonBack(Watchy *watchy) {
  uint16_t state = memory_->state % (itemCount_ + 2);
  switch (state) {
  default:
    if (items_[state - 2]->buttonBack(watchy)) {
      memory_->state  = 1;
      fullDrawNeeded_ = true;
    }
    return false;
  case 1:
    memory_->state  = 0;
    fullDrawNeeded_ = true;
    return false;
  case 0:
    return main_->buttonBack(watchy);
  }
}

bool MenuApp::showMenu(Watchy *watchy, Display *display, bool partialRefresh) {
  display->fillScreen(BACKGROUND_COLOR);
  display->setTextWrap(false);

  LayoutText items[itemCount_];
  LayoutBackground selection;
  LayoutHCenter centeredItems[itemCount_];
  LayoutPad paddedItems[itemCount_];
  LayoutElement *rows[itemCount_ + 2];
  bool vstretch[itemCount_ + 2];

  LayoutText headerText("Menu", TITLE_FONT, FOREGROUND_COLOR);
  LayoutPad paddedHeader(&headerText, 3, 0, 3, 0);
  LayoutHCenter centeredHeader(&paddedHeader);
  LayoutBorder header(&centeredHeader, false, false, true, false,
                      FOREGROUND_COLOR);
  rows[0]     = &header;
  vstretch[0] = false;

  for (int i = 0; i < itemCount_; i++) {
    if (i != memory_->selected) {
      items[i]         = LayoutText(names_[i], FONT, FOREGROUND_COLOR);
      paddedItems[i]   = LayoutPad(&(items[i]), 3, 0, 3, 0);
      centeredItems[i] = LayoutHCenter(&(paddedItems[i]));
      rows[i + 1]      = &(centeredItems[i]);
      vstretch[i + 1]  = false;
    } else {
      items[i]         = LayoutText(names_[i], FONT, BACKGROUND_COLOR);
      paddedItems[i]   = LayoutPad(&(items[i]), 3, 0, 3, 0);
      centeredItems[i] = LayoutHCenter(&(paddedItems[i]));
      selection       = LayoutBackground(&(centeredItems[i]), FOREGROUND_COLOR);
      rows[i + 1]     = &selection;
      vstretch[i + 1] = false;
    }
  }

  LayoutFill fill;
  rows[itemCount_ + 1]     = &fill;
  vstretch[itemCount_ + 1] = true;

  LayoutRows root(itemCount_ + 2, rows, vstretch);
  uint16_t w, h;
  root.draw(display, 0, 0, display->width(), display->height(), &w, &h);
  display->display(partialRefresh);
  return true;
}
