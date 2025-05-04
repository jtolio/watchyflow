#include "Weather.h"
#include "icons.h"
#include "../Watchy/Watchy.h"

void LayoutWeatherIcon::size(Display *display, uint16_t targetWidth,
                             uint16_t targetHeight, uint16_t *width,
                             uint16_t *height) {
  *width  = 48;
  *height = 32;
}

void LayoutWeatherIcon::draw(Display *display, int16_t x0, int16_t y0,
                             uint16_t targetWidth, uint16_t targetHeight,
                             uint16_t *width, uint16_t *height) {
  const unsigned char *weatherIcon = atmosphere;
  if (!upToDate_) {
    weatherIcon = cputemp;
  } else if (weather_ > 801) {
    weatherIcon = cloudy;
  } else if (weather_ == 801) {
    weatherIcon = cloudsun;
  } else if (weather_ == 800) {
    weatherIcon = sunny;
  } else if (weather_ >= 700) {
    weatherIcon = atmosphere;
  } else if (weather_ >= 600) {
    weatherIcon = snow;
  } else if (weather_ >= 500) {
    weatherIcon = rain;
  } else if (weather_ >= 300) {
    weatherIcon = drizzle;
  } else if (weather_ >= 200) {
    weatherIcon = thunderstorm;
  }

  LayoutBitmap elem(weatherIcon, 48, 32, color_);
  elem.draw(display, x0, y0, targetWidth, targetHeight, width, height);
}
