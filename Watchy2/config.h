#ifndef CONFIG_H
#define CONFIG_H

// pins

#ifdef ARDUINO_ESP32S3_DEV // V3

#define WATCHY_V3_SDA 12
#define WATCHY_V3_SCL 11

#define WATCHY_V3_SS   33
#define WATCHY_V3_MOSI 48
#define WATCHY_V3_MISO 46
#define WATCHY_V3_SCK  47

#define MENU_BTN_PIN 7
#define BACK_BTN_PIN 6
#define UP_BTN_PIN   0
#define DOWN_BTN_PIN 8

#define DISPLAY_CS      33
#define DISPLAY_DC      34
#define DISPLAY_RES     35
#define DISPLAY_BUSY    36
#define ACC_INT_1_PIN   14
#define ACC_INT_2_PIN   13
#define VIB_MOTOR_PIN   17
#define BATT_ADC_PIN    9
#define CHRG_STATUS_PIN 10
#define USB_DET_PIN     21
#define RTC_INT_PIN     -1 // not used

#define MENU_BTN_MASK (BIT64(7))
#define BACK_BTN_MASK (BIT64(6))
#define UP_BTN_MASK   (BIT64(0))
#define DOWN_BTN_MASK (BIT64(8))
#define ACC_INT_MASK  (BIT64(14))
#define BTN_PIN_MASK  MENU_BTN_MASK | BACK_BTN_MASK | UP_BTN_MASK | DOWN_BTN_MASK

#else // V1,V1.5,V2

#if !defined(ARDUINO_WATCHY_V10) && !defined(ARDUINO_WATCHY_V15) &&            \
    !defined(ARDUINO_WATCHY_V20)

#pragma message                                                                \
    "Please install the latest ESP32 Arduino Core (2.0.5+) and choose Watchy as the target board"
#pragma message                                                                \
    "Hardware revision is not defined at the project level, please define in config.h. Defaulting to ARDUINO_WATCHY_V20"

#define ARDUINO_WATCHY_V20

#define MENU_BTN_PIN  26
#define BACK_BTN_PIN  25
#define DOWN_BTN_PIN  4
#define DISPLAY_CS    5
#define DISPLAY_RES   9
#define DISPLAY_DC    10
#define DISPLAY_BUSY  19
#define ACC_INT_1_PIN 14
#define ACC_INT_2_PIN 12
#define VIB_MOTOR_PIN 13
#define RTC_INT_PIN   27

#define UP_BTN_PIN   35
#define BATT_ADC_PIN 34
#define UP_BTN_MASK  (BIT64(35))
#define RTC_TYPE     2 // PCF8563

#define MENU_BTN_MASK (BIT64(26))
#define BACK_BTN_MASK (BIT64(25))
#define DOWN_BTN_MASK (BIT64(4))
#define ACC_INT_MASK  (BIT64(14))
#define BTN_PIN_MASK  MENU_BTN_MASK | BACK_BTN_MASK | UP_BTN_MASK | DOWN_BTN_MASK

#endif

#endif

#endif
