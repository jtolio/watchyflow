#pragma once

#include "../../Watchy/Watchy.h"
#include "../../Layout/Arena.h"
#include <vector>
#include <initializer_list>

typedef struct menuAppMemory {
  // state == 0 means main app
  // state == 1 means menu
  // state == n means item n-2
  uint16_t state;
  uint16_t selected;
} menuAppMemory;

class MenuItem {
public:
  explicit MenuItem(String name, WatchyApp *app) : app_(app), name_(name) {}

  friend class MenuApp;

private:
  WatchyApp *app_;
  String name_;
};

extern MemArenaAllocator<MenuItem> allocatorMenuItem;

class MenuApp : public WatchyApp {
public:
  MenuApp(menuAppMemory *memory, WatchyApp *mainFace,
          std::initializer_list<MenuItem> elems);
  MenuApp(menuAppMemory *memory, WatchyApp *mainFace,
          std::vector<MenuItem, MemArenaAllocator<MenuItem>> elems)
      : memory_(memory), main_(mainFace), items_(std::move(elems)),
        fullDrawNeeded_(false) {}

  bool show(Watchy *watchy, Display *display, bool partialRefresh) override;
  bool fetchNetwork(Watchy *watchy) override;

  void reset(Watchy *watchy) override;
  void buttonUp(Watchy *watchy) override;
  void buttonDown(Watchy *watchy) override;
  bool buttonSelect(Watchy *watchy) override;
  bool buttonBack(Watchy *watchy) override;

private:
  bool showMenu(Watchy *watchy, Display *display, bool partialRefresh);

private:
  menuAppMemory *memory_;
  WatchyApp *main_;
  std::vector<MenuItem, MemArenaAllocator<MenuItem>> items_;
  bool fullDrawNeeded_;
};
