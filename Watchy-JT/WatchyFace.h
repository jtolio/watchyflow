#ifndef WATCHYFACE_H
#define WATCHYFACE_H

#include "Watchy.h"

typedef struct weatherData {
  int8_t weatherTemperature;
  int8_t sensorTemperature;
  int16_t weatherConditionCode;  // negative means the weather api failed.
  bool isMetric;
  String weatherDescription;
  tmElements_t sunrise;
  tmElements_t sunset;
} weatherData;

class WatchyFace : public Watchy {
  using Watchy::Watchy;
public:
  void deviceReset() override;
  void postDraw() override;
  void drawWatchFace() override;
private:
  void parseCalendar(String payload);
  weatherData getWeatherData();
  weatherData _getWeatherData(String cityID, String lat, String lon, String units, String lang,
                              String url, String apiKey, uint8_t updateInterval);
};

#endif
