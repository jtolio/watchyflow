#include "MenuApp.h"
#include "../../Layout/Layout.h"
#include "../../Elements/Buttons.h"
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>

namespace {
const GFXfont *TITLE_FONT = &FreeSansBold9pt7b;
const GFXfont *FONT       = &FreeSans9pt7b;
} // namespace

MenuItem::MenuItem(String name, MenuApp *app)
    : app_(app), name_(name), submenu_(true) {}

MenuItem::MenuItem(String name, WatchyApp *app)
    : app_(app), name_(name), submenu_(false) {}

MemArenaAllocator<MenuItem> allocatorMenuItem(globalArena);

MenuApp::MenuApp(menuAppMemory *memory, const char *title,
                 std::initializer_list<MenuItem> elems)
    : memory_(memory), title_(title), items_(allocatorMenuItem),
      fullDrawNeeded_(false) {
  items_.reserve(elems.size());
  for (const MenuItem &info : elems) {
    items_.push_back(info);
  }
}

AppState MenuApp::show(Watchy *watchy, Display *display, bool partialRefresh) {
  if (fullDrawNeeded_) {
    partialRefresh = false;
  }
  fullDrawNeeded_ = false;

  uint16_t index = memory_->index % items_.size();

  if (memory_->inApp) {
    if (items_[index].app_->show(watchy, display, partialRefresh) ==
        APP_ACTIVE) {
      return APP_ACTIVE;
    }
    memory_->inApp = false;
    return APP_EXIT;
  }
  showMenu(watchy, display, partialRefresh);
  return APP_ACTIVE;
}

FetchState MenuApp::fetchNetwork(Watchy *watchy) {
  FetchState fetchState = FETCH_OK;
  for (uint16_t i = 0; i < items_.size(); i++) {
    if (items_[i].app_->fetchNetwork(watchy) == FETCH_TRYAGAIN) {
      fetchState = FETCH_TRYAGAIN;
    }
  }
  return fetchState;
}

void MenuApp::tick(Watchy *watchy) {
  for (uint16_t i = 0; i < items_.size(); i++) {
    items_[i].app_->tick(watchy);
  }
}

void MenuApp::reset(Watchy *watchy) {
  memory_->inApp = false;
  memory_->index = 0;
  for (uint16_t i = 0; i < items_.size(); i++) {
    items_[i].app_->reset(watchy);
  }
}

bool MenuApp::isSubMenu(int index) { return items_[index].submenu_; }

void MenuApp::buttonUp(Watchy *watchy) {
  if (memory_->inApp) {
    items_[memory_->index % items_.size()].app_->buttonUp(watchy);
    return;
  }
  memory_->index = (memory_->index + items_.size() - 1) % items_.size();
}

void MenuApp::buttonDown(Watchy *watchy) {
  if (memory_->inApp) {
    items_[memory_->index % items_.size()].app_->buttonDown(watchy);
    return;
  }
  memory_->index = (memory_->index + 1) % items_.size();
}

AppState MenuApp::buttonSelect(Watchy *watchy) {
  uint16_t index = memory_->index % items_.size();
  bool submenu   = isSubMenu(index);
  if (memory_->inApp) {
    if (items_[index].app_->buttonSelect(watchy) != APP_ACTIVE) {
      memory_->inApp  = false;
      fullDrawNeeded_ = !submenu;
    }
    return APP_ACTIVE;
  }
  memory_->inApp  = true;
  fullDrawNeeded_ = !submenu;
  return APP_ACTIVE;
}

AppState MenuApp::buttonBack(Watchy *watchy) {
  uint16_t index = memory_->index % items_.size();
  bool submenu   = isSubMenu(index);
  if (memory_->inApp) {
    if (items_[index].app_->buttonBack(watchy) != APP_ACTIVE) {
      memory_->inApp  = false;
      fullDrawNeeded_ = !submenu;
    }
    return APP_ACTIVE;
  }
  return APP_EXIT;
}

void MenuApp::showMenu(Watchy *watchy, Display *display, bool partialRefresh) {
  const uint16_t FOREGROUND_COLOR = watchy->foregroundColor();
  const uint16_t BACKGROUND_COLOR = watchy->backgroundColor();

  display->fillScreen(BACKGROUND_COLOR);
  display->setTextWrap(false);

  std::vector<LayoutEntry, MemArenaAllocator<LayoutEntry>> menu(
      allocatorLayoutEntry);
  menu.reserve(items_.size() + 2);

  menu.push_back(LayoutEntry(LayoutBorder(
      LayoutHCenter(LayoutPad(LayoutText(title_, TITLE_FONT, FOREGROUND_COLOR),
                              3, 0, 3, 0)),
      false, false, true, false, FOREGROUND_COLOR)));

  for (int i = 0; i < items_.size(); i++) {
    if (i != memory_->index) {
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

  LayoutButtonLabels(watchy, "Back", "Select", "->", "<-", NULL,
                     FOREGROUND_COLOR, true, LayoutRows(menu))
      .draw(display, 0, 0, display->width(), display->height(), &w, &h);
  display->display(partialRefresh);
}
