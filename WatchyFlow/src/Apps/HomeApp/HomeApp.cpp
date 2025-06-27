#include "HomeApp.h"

AppState HomeApp::show(Watchy *watchy, Display *display) {
  if (!memory_->homeApp) {
    if (menu_->show(watchy, display) == APP_ACTIVE) {
      return APP_ACTIVE;
    }
    memory_->homeApp = true;
  }
  return home_->show(watchy, display);
}

FetchState HomeApp::fetchNetwork(Watchy *watchy) {
  FetchState fetchState = home_->fetchNetwork(watchy);
  if (menu_->fetchNetwork(watchy) == FETCH_TRYAGAIN) {
    fetchState = FETCH_TRYAGAIN;
  }
  return fetchState;
}

void HomeApp::tick(Watchy *watchy) {
  home_->tick(watchy);
  menu_->tick(watchy);
}

void HomeApp::reset(Watchy *watchy) {
  memory_->homeApp = true;
  home_->reset(watchy);
  menu_->reset(watchy);
}

void HomeApp::buttonUp(Watchy *watchy) {
  (memory_->homeApp ? home_ : menu_)->buttonUp(watchy);
}

void HomeApp::buttonDown(Watchy *watchy) {
  (memory_->homeApp ? home_ : menu_)->buttonDown(watchy);
}

AppState HomeApp::buttonSelect(Watchy *watchy) {
  if (!memory_->homeApp) {
    if (menu_->buttonSelect(watchy) != APP_ACTIVE) {
      memory_->homeApp = true;
    }
  } else {
    memory_->homeApp = false;
  }
  return APP_ACTIVE;
}

AppState HomeApp::buttonBack(Watchy *watchy) {
  if (!memory_->homeApp) {
    if (menu_->buttonBack(watchy) != APP_ACTIVE) {
      memory_->homeApp = true;
    }
    return APP_ACTIVE;
  }
  return home_->buttonBack(watchy);
}
