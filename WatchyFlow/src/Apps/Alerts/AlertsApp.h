#pragma once

#include "../../Watchy/WatchyApp.h"

// AlertsApp is an App wrapper which will take over the display and show
// any alerts stored until acknowledged.
class AlertsApp : public WatchyApp {
public:
  AlertsApp() : app_(NULL) {}
  explicit AlertsApp(WatchyApp *wrapped) : app_(wrapped) {}

  AppState show(Watchy *watchy, Display *display, bool partialRefresh) override;
  FetchState fetchNetwork(Watchy *watchy) override {
    return app_->fetchNetwork(watchy);
  }

  void reset(Watchy *watchy) override;
  void buttonUp(Watchy *watchy) override;
  void buttonDown(Watchy *watchy) override;
  AppState buttonSelect(Watchy *watchy) override;
  AppState buttonBack(Watchy *watchy) override;

  void tick(Watchy *watchy) override;

  void addAlert(String summary, time_t alertTime);
  void setApp(WatchyApp *app) { app_ = app; }

private:
  WatchyApp *app_;
};
