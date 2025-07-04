#pragma once

#include "../../Watchy/WatchyApp.h"
#include "../../Layout/Arena.h"
#include <vector>
#include <initializer_list>

typedef struct menuAppMemory {
  bool inApp;
  uint16_t index;
} menuAppMemory;

class MenuApp;

class MenuItem {
public:
  explicit MenuItem(String name, MenuApp *app);
  explicit MenuItem(String name, WatchyApp *app);

  friend class MenuApp;

private:
  WatchyApp *app_;
  String name_;
  bool submenu_;
};

extern MemArenaAllocator<MenuItem> allocatorMenuItem;

// MenuApp will take a list of WatchyApps and names and provide a new higher
// level WatchyApp that keeps track of which app is active and lets the user
// select between them. If the App becomes inactive during the show() call,
// the MenuApp will also become inactive, allowing fast travel out of deep
// menus when an action app (like the app that resets the step counter) is
// triggered.
class MenuApp : public WatchyApp {
public:
  MenuApp(menuAppMemory *memory, const char *title,
          std::initializer_list<MenuItem> elems);
  MenuApp(menuAppMemory *memory, const char *title,
          std::vector<MenuItem, MemArenaAllocator<MenuItem>> elems)
      : memory_(memory), title_(title), items_(std::move(elems)) {}

  AppState show(Watchy *watchy, Display *display) override;
  FetchState fetchNetwork(Watchy *watchy) override;

  void reset(Watchy *watchy) override;
  void buttonUp(Watchy *watchy) override;
  void buttonDown(Watchy *watchy) override;
  AppState buttonSelect(Watchy *watchy) override;
  AppState buttonBack(Watchy *watchy) override;

  void tick(Watchy *watchy) override;

private:
  void showMenu(Watchy *watchy, Display *display);
  bool isSubMenu(int index);

private:
  menuAppMemory *memory_;
  const char *title_;
  std::vector<MenuItem, MemArenaAllocator<MenuItem>> items_;
};
