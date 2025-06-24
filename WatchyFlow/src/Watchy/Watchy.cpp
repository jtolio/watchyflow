#include "Watchy.h"

#include <Arduino.h>
#include <HTTPClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Arduino_JSON.h>
#include <Wire.h>
#include "BLE.h"
#include "bma.h"
#include "config.h"
#include "esp_chip_info.h"
#ifdef ARDUINO_ESP32S3_DEV
#include "Watchy32KRTC.h"
#include "soc/rtc.h"
#include "soc/rtc_io_reg.h"
#include "soc/sens_reg.h"
#include "esp_sleep.h"
#include "rom/rtc.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "time.h"
#include "esp_sntp.h"
#include "hal/rtc_io_types.h"
#include "driver/rtc_io.h"
#define uS_TO_S_FACTOR                                                         \
  1000000ULL // Conversion factor for micro seconds to seconds
#define ADC_VOLTAGE_DIVIDER                                                    \
  ((360.0f + 100.0f) / 360.0f) // Voltage divider at battery ADC
#else
#include "WatchyRTC.h"
#endif

#include "../Layout/Layout.h"
#include "WatchyApp.h"

namespace {
#ifdef ARDUINO_ESP32S3_DEV
Watchy32KRTC rtc_;
#define ACTIVE_LOW 0
#else
WatchyRTC rtc_;
#define ACTIVE_LOW 1
#endif

GxEPD2_BW<WatchyDisplay, WatchyDisplay::HEIGHT> display_(WatchyDisplay{});

RTC_DATA_ATTR BMA423 sensor_;
RTC_DATA_ATTR bool usbPluggedIn_;
RTC_DATA_ATTR time_t lastFetchAttempt_;
RTC_DATA_ATTR time_t lastSuccessfulNetworkFetch_;
RTC_DATA_ATTR uint8_t fetchTries_;
RTC_DATA_ATTR time_t timezoneOffset_;
RTC_DATA_ATTR int lastSuccessfulWiFiIndex_;
RTC_DATA_ATTR uint8_t lastMinute_;
RTC_DATA_ATTR uint32_t totalSteps_;
} // namespace

void _sensorSetup();

void Watchy::sleep() {
  display_.hibernate();
  rtc_.clearAlarm(); // resets the alarm flag in the RTC
#ifdef ARDUINO_ESP32S3_DEV
  esp_sleep_enable_ext0_wakeup(
      (gpio_num_t)USB_DET_PIN,
      usbPluggedIn_ ? LOW
                    : HIGH); //// enable deep sleep wake on USB plug in/out
  rtc_gpio_set_direction((gpio_num_t)USB_DET_PIN, RTC_GPIO_MODE_INPUT_ONLY);
  rtc_gpio_pullup_en((gpio_num_t)USB_DET_PIN);

  esp_sleep_enable_ext1_wakeup(
      BTN_PIN_MASK,
      ESP_EXT1_WAKEUP_ANY_LOW); // enable deep sleep wake on button press
  rtc_gpio_set_direction((gpio_num_t)UP_BTN_PIN, RTC_GPIO_MODE_INPUT_ONLY);
  rtc_gpio_pullup_en((gpio_num_t)UP_BTN_PIN);

  rtc_clk_32k_enable(true);
  // rtc_clk_slow_freq_set(RTC_SLOW_FREQ_32K_XTAL);
  struct tm timeinfo;
  getLocalTime(&timeinfo);
  int secToNextMin = 60 - timeinfo.tm_sec;
  esp_sleep_enable_timer_wakeup(secToNextMin * uS_TO_S_FACTOR);
#else
  // Set GPIOs 0-39 to input to avoid power leaking out
  const uint64_t ignore =
      0b11110001000000110000100111000010; // Ignore some GPIOs due to resets
  for (int i = 0; i < GPIO_NUM_MAX; i++) {
    if ((ignore >> i) & 0b1)
      continue;
    pinMode(i, INPUT);
  }
  esp_sleep_enable_ext0_wakeup((gpio_num_t)RTC_INT_PIN,
                               0); // enable deep sleep wake on RTC interrupt
  esp_sleep_enable_ext1_wakeup(
      BTN_PIN_MASK,
      ESP_EXT1_WAKEUP_ANY_HIGH); // enable deep sleep wake on button press
#endif
  esp_deep_sleep_start();
}

bool connectWiFi(WatchySettings settings) {
  for (int i = 0; i < settings.wifiNetworkCount; i++) {
    int idxToUse = (i + lastSuccessfulWiFiIndex_) % settings.wifiNetworkCount;

    if (WL_CONNECT_FAILED == WiFi.begin(settings.wifiNetworks[idxToUse].SSID,
                                        settings.wifiNetworks[idxToUse].Pass)) {
      continue;
    }

    if (WL_CONNECTED != WiFi.waitForConnectResult()) {
      WiFi.mode(WIFI_OFF);
      btStop();
      continue;
    }

    lastSuccessfulWiFiIndex_ = idxToUse;
    return true;
  }
  return false;
}

void Watchy::wakeup(WatchyApp *app, WatchySettings settings) {
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause(); // get wake up reason
#ifdef ARDUINO_ESP32S3_DEV
  Wire.begin(WATCHY_V3_SDA, WATCHY_V3_SCL); // init i2c
#else
  Wire.begin(SDA, SCL); // init i2c
#endif
  rtc_.init();
  display_.epd2.initWatchy();
  display_.cp437(true);
  display_.setFullWindow();
  display_.epd2.asyncPowerOn();

  WakeupReason wakeup_reason_enum = WAKEUP_RESET;

  switch (wakeup_reason) {
#ifdef ARDUINO_ESP32S3_DEV
  case ESP_SLEEP_WAKEUP_TIMER: // RTC Alarm
#else
  case ESP_SLEEP_WAKEUP_EXT0: // RTC Alarm
#endif
    wakeup_reason_enum = WAKEUP_CLOCK;
    break;
  case ESP_SLEEP_WAKEUP_EXT1: // button Press
    wakeup_reason_enum = WAKEUP_BUTTON;
    break;
#ifdef ARDUINO_ESP32S3_DEV
  case ESP_SLEEP_WAKEUP_EXT0: // USB plug in
    pinMode(USB_DET_PIN, INPUT);
    usbPluggedIn_      = (digitalRead(USB_DET_PIN) == 1);
    wakeup_reason_enum = WAKEUP_USB;
    break;
#endif
  default: // reset
    rtc_.config("");
    _sensorSetup();
#ifdef ARDUINO_ESP32S3_DEV
    pinMode(USB_DET_PIN, INPUT);
    usbPluggedIn_ = (digitalRead(USB_DET_PIN) == 1);
#else
    usbPluggedIn_ = false;
#endif
    // For some reason, seems to be enabled on first boot
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);

    lastFetchAttempt_           = 0;
    lastSuccessfulNetworkFetch_ = 0;
    fetchTries_                 = 0;
    timezoneOffset_             = settings.defaultTimezoneOffset;
    lastSuccessfulWiFiIndex_    = 0;
    totalSteps_                 = 0;
    break;
  }

  tmElements_t currentTime;
  rtc_.read(currentTime);
  Watchy watchy(currentTime, wakeup_reason_enum, settings);
  bool partialRefresh = true;

  switch (wakeup_reason) {
#ifdef ARDUINO_ESP32S3_DEV
  case ESP_SLEEP_WAKEUP_TIMER: // RTC Alarm
#else
  case ESP_SLEEP_WAKEUP_EXT0: // RTC Alarm
#endif
    break;
  case ESP_SLEEP_WAKEUP_EXT1: // button Press
  {
    uint64_t wakeupBit = esp_sleep_get_ext1_wakeup_status();
    if (wakeupBit & MENU_BTN_MASK) {
      if (settings.buttonConfig == BUTTONS_SELECT_BACK_RIGHT) {
        app->buttonDown(&watchy);
      } else {
        app->buttonSelect(&watchy);
      }
    } else if (wakeupBit & BACK_BTN_MASK) {
      if (settings.buttonConfig == BUTTONS_SELECT_BACK_RIGHT) {
        app->buttonUp(&watchy);
      } else {
        app->buttonBack(&watchy);
      }
    } else if (wakeupBit & UP_BTN_MASK) {
      if (settings.buttonConfig == BUTTONS_SELECT_BACK_RIGHT) {
        app->buttonBack(&watchy);
      } else {
        app->buttonUp(&watchy);
      }
    } else if (wakeupBit & DOWN_BTN_MASK) {
      if (settings.buttonConfig == BUTTONS_SELECT_BACK_RIGHT) {
        app->buttonSelect(&watchy);
      } else {
        app->buttonDown(&watchy);
      }
    }
  } break;
#ifdef ARDUINO_ESP32S3_DEV
  case ESP_SLEEP_WAKEUP_EXT0: // USB plug in
    break;
#endif
  default: // reset
    app->reset(&watchy);
    partialRefresh = false;
    break;
  }

  if (currentTime.Minute != lastMinute_) {
    lastMinute_ = currentTime.Minute;
    app->tick(&watchy);
  }
  watchy.updateScreen(app, partialRefresh);

  // don't do a network fetch if it's a user event that didn't trigger one.
  if (!watchy.fetchOnButton_ && (wakeup_reason_enum == WAKEUP_BUTTON ||
                                 wakeup_reason_enum == WAKEUP_USB)) {
    return;
  }

  time_t now       = watchy.unixtime();
  time_t staleTime = now - settings.networkFetchIntervalSeconds;

  if (fetchTries_ >= settings.networkFetchTries) {
    // if lastFetchAttempt is in the future, perhaps the timezone
    // just changed, so we don't want to count that.
    if (lastFetchAttempt_ >= staleTime && lastFetchAttempt_ < now) {
      // lastFetchAttempt is newer than staleTime but not in the future.
      // nothing to do.
      return;
    }
    // okay it's been long enough that we should start over on our try counter.
    fetchTries_ = 0;
  }

  lastFetchAttempt_ = now;
  fetchTries_++;

  watchy.drawNotice("Connecting...");

  if (connectWiFi(settings)) {
    watchy.drawNotice("Loading...   ");

    FetchState fetchResult = app->fetchNetwork(&watchy);
    if (syncNTP()) {
      rtc_.read(currentTime);
      watchy.reset(currentTime, WAKEUP_NETFETCH);
      now = watchy.unixtime();
      if (fetchResult == FETCH_OK) {
        lastSuccessfulNetworkFetch_ = now;
        fetchTries_                 = settings.networkFetchTries;
      }
    }

    WiFi.mode(WIFI_OFF);
    btStop();
  }

  watchy.updateScreen(app, true);
}

void Watchy::updateScreen(WatchyApp *app, bool partialRefresh) {
  app->show(this, &display_, partialRefresh);
  queuedVibrate();
}

void Watchy::reset(const tmElements_t &currentTime, WakeupReason wakeup) {
  localtime_ = currentTime;
  unixtime_  = toUnixTime(currentTime);
  wakeup_    = wakeup;
}

void Watchy::queueVibrate(uint8_t intervalMs, uint8_t length) {
  if (int(vibrateIntervalMs_) * int(vibrateLength_) <
      int(intervalMs) * int(length)) {
    vibrateIntervalMs_ = intervalMs;
    vibrateLength_     = length;
  }
}

void Watchy::queuedVibrate() {
  if (vibrateIntervalMs_ > 0 && vibrateLength_ > 0) {
    vibrate(vibrateIntervalMs_, vibrateLength_);
    vibrateIntervalMs_ = 0;
    vibrateLength_     = 0;
  }
}

void Watchy::vibrate(uint8_t intervalMs, uint8_t length) {
  pinMode(VIB_MOTOR_PIN, OUTPUT);
  bool motorOn = false;
  for (int i = 0; i < length; i++) {
    motorOn = !motorOn;
    digitalWrite(VIB_MOTOR_PIN, motorOn);
    delay(intervalMs);
  }
}

float Watchy::battVoltage() {
#ifdef ARDUINO_ESP32S3_DEV
  return analogReadMilliVolts(BATT_ADC_PIN) / 1000.0f * ADC_VOLTAGE_DIVIDER;
#else
  if (rtc_.rtcType == DS3231) {
    return analogReadMilliVolts(BATT_ADC_PIN) / 1000.0f *
           2.0f; // Battery voltage goes through a 1/2 divider.
  } else {
    return analogReadMilliVolts(BATT_ADC_PIN) / 1000.0f * 2.0f;
  }
#endif
}

int Watchy::battPercent() {
  int percent = (battVoltage() - settings_.emptyVoltage) * 100 /
                (settings_.fullVoltage - settings_.emptyVoltage);
  if (percent > 100) {
    percent = 100;
  }
  if (percent < 0) {
    percent = 0;
  }
  return percent;
}

void Watchy::triggerNetworkFetch() {
  lastFetchAttempt_ = 0;
  fetchTries_       = 0;
  fetchOnButton_    = true;
}

void Watchy::setTimezoneOffset(time_t seconds) {
  // see comments in syncNTP and toUnixTime.
  timezoneOffset_ = seconds;
}

time_t Watchy::timezoneOffset() {
  // see comments in syncNTP and toUnixTime.
  return timezoneOffset_;
}

time_t Watchy::toUnixTime(const tmElements_t &local) {
  // the system clock is stored in the local timezone and not UTC unlike most
  // unix systems.
  // the unix timestamp calculation is therefore off by the local timezone
  // offset from UTC, so to fix it we need to subtract the timezoneOffset.
  // note that time_t may be 32 bits, and may have a Y2038 problem. it may
  // only make sense to use time_t for time deltas. with any luck i'll have a
  // new watch that can know about the year 2039 before 2039.
  return makeTime(local) - timezoneOffset_;
}

tmElements_t Watchy::toLocalTime(time_t unix) {
  // see comment in toUnixTime
  tmElements_t local;
  unix += timezoneOffset_;
  breakTime(unix, local);
  return local;
}

bool Watchy::syncNTP() {
  // NTPClient is weird. you ask it for the local time, and then it gives
  // you "epoch time" in local time, which is weird, because epoch time is
  // supposed to always be UTC. c'est la vie. we'll keep this madness
  // contained. syncNTP will set rtc_ to be the local time, given
  // timezoneOffset. see comment in toUnixTime.
  WiFiUDP ntpUDP;
  NTPClient timeClient(ntpUDP, timezoneOffset_);
  timeClient.begin();
  if (!timeClient.forceUpdate()) {
    return false;
  }
  tmElements_t tm;
  breakTime((time_t)timeClient.getEpochTime(), tm);
  rtc_.set(tm);
  return true;
}

time_t Watchy::lastSuccessfulNetworkFetch() {
  return lastSuccessfulNetworkFetch_;
}

void Watchy::drawNotice(char *msg) {
  LayoutBackground notice(
      LayoutBorder(
          LayoutPad(LayoutText(msg, NULL, foregroundColor()), 3, 3, 3, 3), true,
          true, true, true, foregroundColor()),
      backgroundColor());

  uint16_t w, h;
  notice.size(&display_, 0, 0, &w, &h);
  notice.draw(&display_, display_.width() - w - 3, display_.height() - h - 3, 0,
              0, &w, &h);
  display_.display(true);
}

uint32_t Watchy::stepCounter() { return sensor_.getCounter(); }

void Watchy::resetStepCounter() {
  totalSteps_ += sensor_.getCounter();
  sensor_.resetStepCounter();
}

uint32_t Watchy::totalStepCounter() {
  return totalSteps_ + sensor_.getCounter();
}

uint8_t Watchy::temperature() { return sensor_.readTemperature(); }

bool Watchy::accel(AccelData &acc) {
  Accel bmaAcc;
  bool rv = sensor_.getAccel(bmaAcc);
  acc.x   = bmaAcc.x;
  acc.y   = bmaAcc.y;
  acc.z   = bmaAcc.z;
  return rv;
}

uint8_t Watchy::direction() { return sensor_.getDirection(); }

uint16_t _readRegister(uint8_t address, uint8_t reg, uint8_t *data,
                       uint16_t len) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom((uint8_t)address, (uint8_t)len);
  for (uint8_t i = 0; Wire.available() && i < (uint8_t)len; i++) {
    data[i] = Wire.read();
  }
  return 0;
}

uint16_t _writeRegister(uint8_t address, uint8_t reg, uint8_t *data,
                        uint16_t len) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.write(data, len);
  return (0 != Wire.endTransmission());
}

void _sensorSetup() {
  if (!sensor_.begin(_readRegister, _writeRegister, delay)) {
    // failed
    return;
  }

  Acfg cfg;
  cfg.odr       = BMA4_OUTPUT_DATA_RATE_100HZ;
  cfg.range     = BMA4_ACCEL_RANGE_2G;
  cfg.bandwidth = BMA4_ACCEL_NORMAL_AVG4;
  cfg.perf_mode = BMA4_CONTINUOUS_MODE;

  sensor_.setAccelConfig(cfg);
  sensor_.enableAccel();

  struct bma4_int_pin_config config;
  config.edge_ctrl = BMA4_LEVEL_TRIGGER;
  config.lvl       = BMA4_ACTIVE_HIGH;
  config.od        = BMA4_PUSH_PULL;
  config.output_en = BMA4_OUTPUT_ENABLE;
  config.input_en  = BMA4_INPUT_DISABLE;
  sensor_.setINTPinConfig(config, BMA4_INTR1_MAP);

  struct bma423_axes_remap remap_data;
  remap_data.x_axis      = 1;
  remap_data.x_axis_sign = 0xff;
  remap_data.y_axis      = 0;
  remap_data.y_axis_sign = 0xff;
  remap_data.z_axis      = 2;
  remap_data.z_axis_sign = 0xff;
  sensor_.setRemapAxes(&remap_data);

  sensor_.enableFeature(BMA423_STEP_CNTR, true);
  sensor_.enableFeature(BMA423_TILT, true);
  sensor_.enableFeature(BMA423_WAKEUP, true);

  sensor_.resetStepCounter();
  sensor_.enableStepCountInterrupt();
  sensor_.enableTiltInterrupt();
  sensor_.enableWakeupInterrupt();
}

uint16_t Watchy::foregroundColor() const {
  return settings_.darkMode ? GxEPD_WHITE : GxEPD_BLACK;
}

uint16_t Watchy::backgroundColor() const {
  return settings_.darkMode ? GxEPD_BLACK : GxEPD_WHITE;
}
