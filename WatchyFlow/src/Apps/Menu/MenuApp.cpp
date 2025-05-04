#include "MenuApp.h"
#include "../../Layout/Layout.h"
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>

#define DARKMODE false

const GFXfont *TITLE_FONT = &FreeSansBold9pt7b;
const GFXfont *FONT       = &FreeSans9pt7b;

const uint16_t FOREGROUND_COLOR = DARKMODE ? GxEPD_WHITE : GxEPD_BLACK;
const uint16_t BACKGROUND_COLOR = DARKMODE ? GxEPD_BLACK : GxEPD_WHITE;

MemArenaAllocator<MenuItem> allocatorMenuItem(globalArena);

MenuApp::MenuApp(menuAppMemory *memory, WatchyApp *mainFace,
                 std::initializer_list<MenuItem> elems)
    : memory_(memory), main_(mainFace), items_(allocatorMenuItem),
      fullDrawNeeded_(false) {
  items_.reserve(elems.size());
  for (const MenuItem &info : elems) {
    items_.push_back(info);
  }
}

bool MenuApp::show(Watchy *watchy, Display *display, bool partialRefresh) {
  if (fullDrawNeeded_) {
    partialRefresh = false;
  }
  fullDrawNeeded_ = false;

  uint16_t state = memory_->state % (items_.size() + 2);
  if (state > 1) {
    if (items_[state - 2].app_->show(watchy, display, partialRefresh)) {
      return true;
    }
    memory_->state = state = 0;
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
  for (uint16_t i = 0; i < items_.size(); i++) {
    if (!items_[i].app_->fetchNetwork(watchy)) {
      success = false;
    }
  }
  return success;
}

void MenuApp::reset(Watchy *watchy) {
  memory_->state    = 0;
  memory_->selected = 0;
  main_->reset(watchy);
  for (uint16_t i = 0; i < items_.size(); i++) {
    items_[i].app_->reset(watchy);
  }
}

void MenuApp::buttonUp(Watchy *watchy) {
  uint16_t state = memory_->state % (items_.size() + 2);
  switch (state) {
  default:
    items_[state - 2].app_->buttonUp(watchy);
    return;
  case 1:
    memory_->selected = (memory_->selected + items_.size() - 1) % items_.size();
    return;
  case 0:
    main_->buttonUp(watchy);
    return;
  }
}

void MenuApp::buttonDown(Watchy *watchy) {
  uint16_t state = memory_->state % (items_.size() + 2);
  switch (state) {
  default:
    items_[state - 2].app_->buttonDown(watchy);
    return;
  case 1:
    memory_->selected = (memory_->selected + 1) % items_.size();
    return;
  case 0:
    main_->buttonDown(watchy);
    return;
  }
}

bool MenuApp::buttonSelect(Watchy *watchy) {
  uint16_t state    = memory_->state % (items_.size() + 2);
  uint16_t selected = memory_->selected;
  switch (state) {
  default:
    if (items_[state - 2].app_->buttonSelect(watchy)) {
      memory_->state  = 1;
      fullDrawNeeded_ = true;
    }
    return false;
  case 1:
    selected        = selected % items_.size();
    memory_->state  = selected + 2;
    fullDrawNeeded_ = true;
    return false;
  case 0:
    memory_->state  = 1;
    fullDrawNeeded_ = true;
    return false;
  }
}

bool MenuApp::buttonBack(Watchy *watchy) {
  uint16_t state = memory_->state % (items_.size() + 2);
  switch (state) {
  default:
    if (items_[state - 2].app_->buttonBack(watchy)) {
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

  std::vector<LayoutEntry, MemArenaAllocator<LayoutEntry>> menu(
      allocatorLayoutEntry);
  menu.reserve(items_.size() + 2);

  menu.push_back(LayoutEntry(LayoutBorder(
      LayoutHCenter(LayoutPad(LayoutText("Menu", TITLE_FONT, FOREGROUND_COLOR),
                              3, 0, 3, 0)),
      false, false, true, false, FOREGROUND_COLOR)));

  for (int i = 0; i < items_.size(); i++) {
    if (i != memory_->selected) {
      menu.push_back(LayoutEntry(LayoutHCenter(LayoutPad(
          LayoutText(items_[i].name_, FONT, FOREGROUND_COLOR), 3, 0, 3, 0))));
    } else {
      menu.push_back(LayoutEntry(LayoutBackground(
          LayoutHCenter(LayoutPad(
              LayoutText(items_[i].name_, FONT, BACKGROUND_COLOR), 3, 0, 3, 0)),
          FOREGROUND_COLOR)));
    }
  }

  menu.push_back(LayoutEntry(LayoutFill(), true));

  uint16_t w, h;
  LayoutRows(menu).draw(display, 0, 0, display->width(), display->height(), &w,
                        &h);
  display->display(partialRefresh);
  return true;
}
