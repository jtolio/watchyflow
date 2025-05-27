#pragma once

#include "Watchy.h"

typedef enum AppState {
  APP_EXIT   = 0,
  APP_ACTIVE = 1,
} AppState;

typedef enum FetchState {
  FETCH_OK       = 0,
  FETCH_TRYAGAIN = 1,
} FetchState;

class WatchyApp {
public:
  virtual AppState show(Watchy *watchy, Display *display,
                        bool partialRefresh) = 0;

  virtual FetchState fetchNetwork(Watchy *watchy) { return FETCH_OK; }
  virtual void reset(Watchy *watchy) {}

  virtual void buttonUp(Watchy *watchy) {}
  virtual void buttonDown(Watchy *watchy) {}
  virtual AppState buttonSelect(Watchy *watchy) { return APP_ACTIVE; }
  virtual AppState buttonBack(Watchy *watchy) { return APP_EXIT; }

  virtual void tick(Watchy *watchy) {}

  virtual ~WatchyApp() = default;
};
