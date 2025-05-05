#pragma once

typedef struct WiFiConfig {
  String SSID;
  String Pass;
} WiFiConfig;

typedef enum ButtonConfiguration {
  BUTTONS_SELECT_BACK_LEFT  = 0,
  BUTTONS_SELECT_BACK_RIGHT = 1,
} ButtonConfiguration;

typedef struct WatchySettings {
  // number of seconds between network fetch attempts
  int networkFetchIntervalSeconds;
  // number of failing network fetch tries before giving up until the next
  // interval.
  int networkFetchTries;

  WiFiConfig *wifiNetworks;
  int wifiNetworkCount;

  time_t defaultTimezoneOffset;

  ButtonConfiguration buttonConfig;

  float fullVoltage;
  float emptyVoltage;
} WatchySettings;
