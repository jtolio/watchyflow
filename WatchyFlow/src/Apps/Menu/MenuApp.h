#pragma once

#include "../../Watchy/WatchyApp.h"
#include "../../Layout/Arena.h"
#include <vector>
#include <initializer_list>

typedef struct menuAppMemory {
  bool inApp;
  uint16_t index;
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
  MenuApp(menuAppMemory *memory, const char *title,
          std::initializer_list<MenuItem> elems);
  MenuApp(menuAppMemory *memory, const char *title,
          std::vector<MenuItem, MemArenaAllocator<MenuItem>> elems)
      : memory_(memory), title_(title), items_(std::move(elems)),
        fullDrawNeeded_(false) {}

  AppState show(Watchy *watchy, Display *display, bool partialRefresh) override;
  FetchState fetchNetwork(Watchy *watchy) override;

  void reset(Watchy *watchy) override;
  void buttonUp(Watchy *watchy) override;
  void buttonDown(Watchy *watchy) override;
  AppState buttonSelect(Watchy *watchy) override;
  AppState buttonBack(Watchy *watchy) override;

  void tick(Watchy *watchy) override;

private:
  void showMenu(Watchy *watchy, Display *display, bool partialRefresh);

private:
  menuAppMemory *memory_;
  const char *title_;
  std::vector<MenuItem, MemArenaAllocator<MenuItem>> items_;
  bool fullDrawNeeded_;
};
