#ifndef MENU_APP_H
#define MENU_APP_H

#include "Watchy.h"

typedef struct menuAppMemory {
  // state == 0 means main app
  // state == 1 means menu
  // state == n means item n-2
  uint16_t state;
  uint16_t selected;
} menuAppMemory;

class MenuApp : public WatchyApp {
public:
  explicit MenuApp(menuAppMemory *memory, WatchyApp *mainFace,
                   uint16_t itemCount, WatchyApp *items[], String names[])
      : memory_(memory), main_(mainFace), itemCount_(itemCount), items_(items),
        names_(names), fullDrawNeeded_(false) {}

  bool show(Watchy *watchy, Display *display, bool partialRefresh) override;
  bool fetchNetwork(Watchy *watchy) override;

  void reset(Watchy *watchy) override;
  void buttonUp(Watchy *watchy) override;
  void buttonDown(Watchy *watchy) override;
  void buttonSelect(Watchy *watchy) override;
  bool buttonBack(Watchy *watchy) override;

private:
  bool showMenu(Watchy *watchy, Display *display, bool partialRefresh);

private:
  menuAppMemory *memory_;
  WatchyApp *main_;
  uint16_t itemCount_;
  WatchyApp **items_;
  String *names_;
  bool fullDrawNeeded_;
};

#endif
