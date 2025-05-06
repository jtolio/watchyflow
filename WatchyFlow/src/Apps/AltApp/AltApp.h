#pragma once

#include "../../Watchy/WatchyApp.h"

typedef struct altAppMemory {
  bool altApp;
} altAppMemory;

class AltApp : public WatchyApp {
public:
  AltApp(altAppMemory *memory, WatchyApp *main, WatchyApp *alt)
      : memory_(memory), main_(main), alt_(alt), fullDrawNeeded_(false) {}

  AppState show(Watchy *watchy, Display *display, bool partialRefresh) override;
  FetchState fetchNetwork(Watchy *watchy) override;

  void reset(Watchy *watchy) override;
  void buttonUp(Watchy *watchy) override;
  void buttonDown(Watchy *watchy) override;
  AppState buttonSelect(Watchy *watchy) override;
  AppState buttonBack(Watchy *watchy) override;

private:
  altAppMemory *memory_;
  WatchyApp *main_;
  WatchyApp *alt_;
  bool fullDrawNeeded_;
};
