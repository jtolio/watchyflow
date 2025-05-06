#include "AltApp.h"

AppState AltApp::show(Watchy *watchy, Display *display, bool partialRefresh) {
  if (fullDrawNeeded_) {
    partialRefresh = false;
  }
  fullDrawNeeded_ = false;

  if (memory_->altApp) {
    if (alt_->show(watchy, display, partialRefresh) == APP_ACTIVE) {
      return APP_ACTIVE;
    }
    memory_->altApp = false;
    partialRefresh  = false;
  }
  return main_->show(watchy, display, partialRefresh);
}

FetchState AltApp::fetchNetwork(Watchy *watchy) {
  FetchState fetchState = main_->fetchNetwork(watchy);
  if (alt_->fetchNetwork(watchy) == FETCH_TRYAGAIN) {
    fetchState = FETCH_TRYAGAIN;
  }
  return fetchState;
}

void AltApp::reset(Watchy *watchy) {
  memory_->altApp = false;
  main_->reset(watchy);
  alt_->reset(watchy);
}

void AltApp::buttonUp(Watchy *watchy) {
  (memory_->altApp ? alt_ : main_)->buttonUp(watchy);
}

void AltApp::buttonDown(Watchy *watchy) {
  (memory_->altApp ? alt_ : main_)->buttonDown(watchy);
}

AppState AltApp::buttonSelect(Watchy *watchy) {
  if (memory_->altApp) {
    if (alt_->buttonSelect(watchy) != APP_ACTIVE) {
      memory_->altApp = false;
      fullDrawNeeded_ = true;
    }
  } else {
    memory_->altApp = true;
    fullDrawNeeded_ = true;
  }
  return APP_ACTIVE;
}

AppState AltApp::buttonBack(Watchy *watchy) {
  if (memory_->altApp) {
    if (alt_->buttonBack(watchy) != APP_ACTIVE) {
      memory_->altApp = false;
      fullDrawNeeded_ = true;
    }
    return APP_ACTIVE;
  }
  return main_->buttonBack(watchy);
}
