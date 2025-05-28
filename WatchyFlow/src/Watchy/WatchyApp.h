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

// WatchyApp is the basic unit of logic for display on a Watchy. WatchyApps
// can be complex collections of sub apps and sub logic. There are many
// example Apps in the Apps/ folder.
class WatchyApp {
public:
  // Every WatchyApp must implement this. It must return an AppState. If the
  // App is active, it is assumed to have drawn something to the display.
  // If the App has exited, it is a signal to a higher app that it should take
  // over. Note that if the root app exits, it will continue to be displayed.
  // Active apps are expected to call display->display(). If partialRefresh
  // is false, the display->display() call should do a full draw. If
  // partialRefresh is true, then the call to display->display() may do a
  // partial draw if it chooses.
  virtual AppState show(Watchy *watchy, Display *display,
                        bool partialRefresh) = 0;

  // fetchNetwork is called when an active WiFi connection is already
  // established (usually once an hour). If the app has network operations it
  // needs to perform from time to time, it should do so in this call. Returning
  // FETCH_TRYAGAIN means that the call failed, and fetchNetwork would like
  // the Watchy to try again reasonably soon. show will be called after this
  // call.
  virtual FetchState fetchNetwork(Watchy *watchy) { return FETCH_OK; }

  // tick() is called once a minute and calls to tick should be passed through
  // to all child apps, active or no. Where show() is only called if the app
  // is active, tick() should be called so apps can do background bookkeeping
  // or trigger alarms.
  virtual void tick(Watchy *watchy) {}

  // reset is called whenever the watchy is initialized. If your app has
  // RTC_DATA, it should be zeroed in this call.
  virtual void reset(Watchy *watchy) {}

  // These methods are called when buttons are pressed. show() will be called
  // after these calls. buttonSelect and buttonBack should return the current
  // app state.
  virtual void buttonUp(Watchy *watchy) {}
  virtual void buttonDown(Watchy *watchy) {}
  virtual AppState buttonSelect(Watchy *watchy) { return APP_ACTIVE; }
  virtual AppState buttonBack(Watchy *watchy) { return APP_EXIT; }

  virtual ~WatchyApp() = default;
};
