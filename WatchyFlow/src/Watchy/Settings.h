#pragma once

typedef struct WiFiConfig {
  String SSID;
  String Pass;
} WiFiConfig;

typedef enum ButtonConfiguration {
  // Should the select and back buttons be on the left?
  // (default, up and down on the right))
  BUTTONS_SELECT_BACK_LEFT = 0,

  // Should the select and back buttons be on the right?
  // (up and down on the left)
  BUTTONS_SELECT_BACK_RIGHT = 1,
} ButtonConfiguration;

// see settings.h.example for an example and more docs.
typedef struct WatchySettings {
  // number of seconds between network fetch attempts
  int networkFetchIntervalSeconds;

  // number of failing network fetch tries before giving up until the next
  // interval.
  int networkFetchTries;

  // what wifi networks to try
  WiFiConfig *wifiNetworks;
  int wifiNetworkCount;

  // if nothing sets this, what should we use by default? (the timezone offset
  // can be programatically set by apps and services that make api calls).
  time_t defaultTimezoneOffset;

  // how to orient the buttons?
  ButtonConfiguration buttonConfig;

  // what voltage above which is the battery considered 100%?
  float fullVoltage;
  // what voltage below which is the battery considered 0%?
  float emptyVoltage;
} WatchySettings;
