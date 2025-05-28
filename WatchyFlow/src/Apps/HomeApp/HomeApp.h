#pragma once

#include "../../Watchy/WatchyApp.h"

typedef struct homeAppMemory {
  bool homeApp;
} homeAppMemory;

// HomeApp is an app that shows one app and will toggle to another app when
// the select button is pressed. If the "menu" alternate app becomes inactive,
// the home app will become active again.
class HomeApp : public WatchyApp {
public:
  HomeApp(homeAppMemory *memory, WatchyApp *home, WatchyApp *menu)
      : memory_(memory), home_(home), menu_(menu), fullDrawNeeded_(false) {}

  AppState show(Watchy *watchy, Display *display, bool partialRefresh) override;
  FetchState fetchNetwork(Watchy *watchy) override;

  void reset(Watchy *watchy) override;
  void buttonUp(Watchy *watchy) override;
  void buttonDown(Watchy *watchy) override;
  AppState buttonSelect(Watchy *watchy) override;
  AppState buttonBack(Watchy *watchy) override;
  void tick(Watchy *watchy) override;

private:
  homeAppMemory *memory_;
  WatchyApp *home_;
  WatchyApp *menu_;
  bool fullDrawNeeded_;
};
