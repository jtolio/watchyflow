#include "Watchy.h"

#include <Arduino.h>
#include <WiFiManager.h>
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

#ifdef ARDUINO_ESP32S3_DEV
Watchy32KRTC rtc_;
#define ACTIVE_LOW 0
#else
WatchyRTC rtc_;
#define ACTIVE_LOW 1
#endif

GxEPD2_BW<WatchyDisplay, WatchyDisplay::HEIGHT> display_(WatchyDisplay{});
RTC_DATA_ATTR bool usbPluggedIn_;

RTC_DATA_ATTR time_t lastFetchAttempt;
RTC_DATA_ATTR uint8_t fetchTries;

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

  switch (wakeup_reason) {
#ifdef ARDUINO_ESP32S3_DEV
  case ESP_SLEEP_WAKEUP_TIMER: // RTC Alarm
#else
  case ESP_SLEEP_WAKEUP_EXT0: // RTC Alarm
#endif
    break;
  case ESP_SLEEP_WAKEUP_EXT1: // button Press
    break;
#ifdef ARDUINO_ESP32S3_DEV
  case ESP_SLEEP_WAKEUP_EXT0: // USB plug in
    pinMode(USB_DET_PIN, INPUT);
    usbPluggedIn_ = (digitalRead(USB_DET_PIN) == 1);
    break;
#endif
  default: // reset
    rtc_.config("");
#ifdef ARDUINO_ESP32S3_DEV
    pinMode(USB_DET_PIN, INPUT);
    usbPluggedIn_    = (digitalRead(USB_DET_PIN) == 1);
    lastFetchAttempt = 0;
    fetchTries       = 0;
#endif
    // For some reason, seems to be enabled on first boot
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
    break;
  }

  tmElements_t currentTime;
  rtc_.read(currentTime);
  Watchy watchy(currentTime);
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
      app->buttonSelect(&watchy);
    } else if (wakeupBit & BACK_BTN_MASK) {
      app->buttonBack(&watchy);
    } else if (wakeupBit & UP_BTN_MASK) {
      app->buttonUp(&watchy);
    } else if (wakeupBit & DOWN_BTN_MASK) {
      app->buttonDown(&watchy);
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

  app->show(&watchy, &display_);
  display_.display(partialRefresh);

  time_t now       = makeTime(currentTime);
  time_t staleTime = now - settings.networkFetchIntervalSeconds;

  if (fetchTries >= settings.networkFetchTries) {
    if (lastFetchAttempt >= staleTime) {
      return;
    }
    fetchTries = 0;
  }

  lastFetchAttempt = now;
  fetchTries++;

  if (WL_CONNECT_FAILED == WiFi.begin(settings.wifiSSID, settings.wifiPass)) {
    return;
  }
  if (WL_CONNECTED != WiFi.waitForConnectResult()) {
    WiFi.mode(WIFI_OFF);
    btStop();
    return;
  }

  if (app->fetchNetwork(&watchy)) {
    fetchTries = settings.networkFetchTries;
  }

  WiFi.mode(WIFI_OFF);
  btStop();

  app->show(&watchy, &display_);
  display_.display(true);
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

void Watchy::triggerNetworkFetch() {
  lastFetchAttempt = 0;
  fetchTries       = 0;
}
