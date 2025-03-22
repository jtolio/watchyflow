#ifndef WATCHYFACE_H
#define WATCHYFACE_H

#include "Watchy.h"

typedef struct weatherData {
  int8_t weatherTemperature;
  int16_t weatherConditionCode;  // negative means the weather api failed.
  bool isMetric;
} weatherData;

class WatchyFace : public Watchy {
  using Watchy::Watchy;
public:
  void deviceReset() override;
  void postDraw() override;
  void drawWatchFace() override;
private:
  void parseCalendar(String payload);
  void fetchWeather();
  void fetchCalendar();
};

#endif
