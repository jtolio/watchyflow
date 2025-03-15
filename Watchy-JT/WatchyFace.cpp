#include "WatchyFace.h"

#define DARKMODE false

const uint8_t BATTERY_SEGMENT_WIDTH = 7;
const uint8_t BATTERY_SEGMENT_HEIGHT = 11;
const uint8_t BATTERY_SEGMENT_SPACING = 9;
const uint8_t WEATHER_ICON_WIDTH = 48;
const uint8_t WEATHER_ICON_HEIGHT = 32;

void WatchyFace::drawWatchFace() {
    display.fillScreen(DARKMODE ? GxEPD_BLACK : GxEPD_WHITE);
    display.setTextColor(DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
    // 200, 200

    drawTime(10, 5);
    drawBattery(200-5, 5);
    int16_t dateW = (int16_t)drawDate(5, 25+5+5);
    drawWeather();
    drawCal(5 + dateW + 5, 25+5+5);
}

void WatchyFace::drawTime(int16_t x0, int16_t y0) {
    display.setFont(&DSEG7_Classic_Bold_25);

    String timeStr = "";
    int displayHour = ((currentTime.Hour+11)%12)+1;
    if (displayHour < 10){
      timeStr += "0";
    }
    timeStr += String(displayHour) + ":";
    if (currentTime.Minute < 10){
      timeStr += "0";
    }
    timeStr += String(currentTime.Minute);

    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(timeStr, 0, 0, &x1, &y1, &w, &h);

    display.setCursor(x0 - x1, y0 - y1);
    display.print(timeStr);
}

uint16_t WatchyFace::drawDate(int16_t x0, int16_t y0) {

    int16_t  dayx1, monx1, numx1, dayy1, mony1, numy1;
    uint16_t dayW, dayH;
    uint16_t monW, monH;
    uint16_t numW, numH;
    uint16_t maxW;

    display.setFont(&Seven_Segment10pt7b);
    String dayOfWeek = dayShortStr(currentTime.Wday);
    display.getTextBounds(dayOfWeek, 0, 0, &dayx1, &dayy1, &dayW, &dayH);
    String month = monthShortStr(currentTime.Month);
    display.getTextBounds(month, 0, 0, &monx1, &mony1, &monW, &monH);

    //display.setFont(&DSEG7_Classic_Bold_25);
    String dayNum = String(currentTime.Day);
    if (currentTime.Day < 10) {
      dayNum = String("0") + dayNum;
    }
    display.getTextBounds(dayNum, 0, 0, &numx1, &numy1, &numW, &numH);

    maxW = dayW;
    if (maxW < monW) {
      maxW = monW;
    }
    if (maxW < numW) {
      maxW = numW;
    }

    display.setFont(&Seven_Segment10pt7b);
    display.setCursor(x0 + (maxW - dayW) - dayx1, y0 - dayy1);
    display.println(dayOfWeek);
    display.setCursor(x0 + (maxW - monW) - monx1, y0 - mony1 + dayH + 2);
    display.println(month);

    //display.setFont(&DSEG7_Classic_Bold_25);
    display.setCursor(x0 + (maxW - numW) - numx1, y0 - numy1 + dayH + monH + 6);
    display.println(dayNum);

    return maxW;
}

void WatchyFace::drawBattery(int16_t x1, int16_t y0) {    
    display.drawBitmap(x1 - 37, y0, battery, 37, 21, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
    display.fillRect(x1 - 32, y0 + 5, 27, BATTERY_SEGMENT_HEIGHT, DARKMODE ? GxEPD_BLACK : GxEPD_WHITE);
    int8_t batteryLevel = 0;
    float VBAT = getBatteryVoltage();
    if(VBAT > 4.0){
        batteryLevel = 3;
    }
    else if(VBAT > 3.6 && VBAT <= 4.0){
        batteryLevel = 2;
    }
    else if(VBAT > 3.20 && VBAT <= 3.6){
        batteryLevel = 1;
    }
    else if(VBAT <= 3.20){
        batteryLevel = 0;
    }

    for(int8_t batterySegments = 0; batterySegments < batteryLevel; batterySegments++){
        display.fillRect(x1 - 32 + (batterySegments * BATTERY_SEGMENT_SPACING), y0 + 5, BATTERY_SEGMENT_WIDTH, BATTERY_SEGMENT_HEIGHT, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
    }
}

void WatchyFace::drawWeather() {
    weatherData currentWeather = getWeatherData();

    int8_t weatherTemperature = currentWeather.weatherTemperature;
    int16_t weatherConditionCode = currentWeather.weatherConditionCode;

    if (weatherConditionCode < 0) {
      display.drawBitmap(200-5-37-5-26, 5, WIFI_CONFIGURED ? wifi : wifioff, 26, 18, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
    } else {
      int16_t  x1, y1;
      uint16_t w, h;
      display.setFont(&Seven_Segment10pt7b);
      String weatherStr = String(weatherTemperature) + (currentWeather.isMetric ? "C" : "F");
      display.getTextBounds(weatherStr, 0, 0, &x1, &y1, &w, &h);
      display.setCursor(200-5-37-5 - w - x1, 5 - y1 + 9 - (h/2));
      display.println(weatherStr);

      /*
      const unsigned char* weatherIcon = 0;

      //https://openweathermap.org/weather-conditions
      if(weatherConditionCode > 801){//Cloudy
        weatherIcon = cloudy;
      }else if(weatherConditionCode == 801){//Few Clouds
        weatherIcon = cloudsun;
      }else if(weatherConditionCode == 800){//Clear
        weatherIcon = sunny;
      }else if(weatherConditionCode >=700){//Atmosphere
        weatherIcon = atmosphere;
      }else if(weatherConditionCode >=600){//Snow
        weatherIcon = snow;
      }else if(weatherConditionCode >=500){//Rain
        weatherIcon = rain;
      }else if(weatherConditionCode >=300){//Drizzle
        weatherIcon = drizzle;
      }else if(weatherConditionCode >=200){//Thunderstorm
        weatherIcon = thunderstorm;
      }

      if (weatherIcon != 0) {
        display.drawBitmap(145, 158, weatherIcon, WEATHER_ICON_WIDTH, WEATHER_ICON_HEIGHT, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
      }
      */
    }
}

void WatchyFace::drawCal(int16_t x0, int16_t y0) {
  display.drawRect(x0, y0, 200-5-x0, 200 - 5 - y0, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
}
