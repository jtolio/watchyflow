#include "Calendar.h"
#include "Layout.h"
#include "Watchy.h"
#include <Fonts/Picopixel.h>
#include <Fonts/FreeMono9pt7b.h>

const GFXfont *SMALL_FONT          = &Picopixel;
const time_t CALENDAR_PAST_SECONDS = 30 * 60;
const time_t SMALLEST_EVENT        = 30 * 60;
const int16_t EVENT_PADDING        = 2;
const time_t SECONDS_PER_PIXEL =
    SMALLEST_EVENT /
    ((int32_t)(SMALL_FONT->yAdvance - 1) + (2 * EVENT_PADDING));

// TODO
time_t gmtOffset = -4 * 60 * 60;

time_t unixEpochTime(tmElements_t tm) {
  // the system clock is stored in the local timezone and not UTC, like most
  // unix systems.
  // the unix timestamp calculation is therefore off by the local timezone
  // offset from UTC, so to fix it we need to subtract the gmtOffset.
  // note that time_t may be 32 bits, and may have a Y2038 problem. it may
  // only make sense to use time_t for time deltas.
  return makeTime(tm) - gmtOffset;
}

void fromUnixEpochTime(time_t ts, tmElements_t *tm) {
  // see comment in unixEpochTime
  ts += gmtOffset;
  breakTime(ts, *tm);
}

void reset(dayEventsData *data) { data->eventCount = 0; }

void addEvent(dayEventsData *data, String summary, String start, String end) {
  if (data->eventCount >= MAX_DAY_EVENTS) {
    return;
  }
  if (data->eventCount == MAX_DAY_EVENTS - 1) {
    summary = "TOO MANY EVENTS";
  }
  start.toCharArray(data->events[data->eventCount].start, 11);
  end.toCharArray(data->events[data->eventCount].end, 11);
  summary.toCharArray(data->events[data->eventCount].summary,
                      MAX_EVENT_NAME_LEN);
  data->eventCount++;
}

void reset(eventsData *data) { data->eventCount = 0; }

void addEvent(eventsData *data, String summary, time_t start, time_t end) {
  if (data->eventCount >= MAX_EVENTS_PER_COLUMN) {
    return;
  }
  if (data->eventCount == MAX_EVENTS_PER_COLUMN - 1) {
    summary = "TOO MANY EVENTS";
  }
  data->events[data->eventCount].start = start;
  data->events[data->eventCount].end   = end;
  summary.toCharArray(data->events[data->eventCount].summary,
                      MAX_EVENT_NAME_LEN);
  data->eventCount++;
}

void reset(alarmsData *data) { data->alarmCount = 0; }

void addAlarm(alarmsData *data, String summary, time_t start) {
  if (data->alarmCount >= MAX_ALARMS) {
    return;
  }
  if (data->alarmCount == MAX_ALARMS - 1) {
    summary = "TOO MANY ALARMS";
  }
  data->alarms[data->alarmCount].start = start;
  summary.toCharArray(data->alarms[data->alarmCount].summary,
                      MAX_EVENT_NAME_LEN);
  data->alarmCount++;
}

void CalendarDayEvents::maybeDraw(Display *display, int16_t x0, int16_t y0,
                                  uint16_t targetWidth, uint16_t targetHeight,
                                  uint16_t *width, uint16_t *height,
                                  bool noop) {
  *width  = targetWidth;
  *height = 0;

  display->setFont(SMALL_FONT);
  display->setTextColor(color_);

  String today = String(tmYearToCalendar(currentTime_.Year));
  today += (currentTime_.Month < 10) ? "-0" : "-";
  today += String(currentTime_.Month);
  today += (currentTime_.Day < 10) ? "-0" : "-";
  today += String(currentTime_.Day);

  for (int i = 0; i < data_->eventCount; i++) {
    String text = data_->events[i].summary;
    if (String(data_->events[i].start) > today ||
        String(data_->events[i].end) <= today) {
      continue;
    }

    uint16_t textWidth, textHeight;
    int16_t x1, y1;
    display->getTextBounds(text, 0, 0, &x1, &y1, &textWidth, &textHeight);
    textWidth += EVENT_PADDING * 2;
    textHeight += EVENT_PADDING;
    if (textWidth > *width) {
      *width = textWidth;
    }
    if (!noop) {
      display->setCursor(x0 - x1 + EVENT_PADDING,
                         y0 - y1 + *height + EVENT_PADDING);
      display->print(text);
    }
    *height += textHeight;
  }

  if (*height > 0) {
    *height += EVENT_PADDING;
    if (!noop) {
      display->drawFastHLine(x0, y0 + *height, *width, color_);
    }
  }
}

void CalendarColumn::draw(Display *display, int16_t x0, int16_t y0,
                          uint16_t targetWidth, uint16_t targetHeight,
                          uint16_t *width, uint16_t *height) {
  if (targetHeight <= 0) {
    targetHeight = 1;
  }

  *width  = targetWidth;
  *height = targetHeight;

  display->setFont(SMALL_FONT);
  display->setTextColor(color_);

  time_t now         = unixEpochTime(currentTime_);
  time_t windowStart = now - CALENDAR_PAST_SECONDS;
  time_t windowEnd   = windowStart + (targetHeight * SECONDS_PER_PIXEL);

  for (int i = 0; i < data_->eventCount; i++) {
    eventData *event  = &(data_->events[i]);
    time_t eventStart = event->start;
    time_t eventEnd   = event->end;
    if (eventEnd <= windowStart) {
      continue;
    }
    if (eventStart >= windowEnd) {
      continue;
    }

    tmElements_t eventStarttm;
    fromUnixEpochTime(eventStart, &eventStarttm);
    if (eventStarttm.Hour == currentTime_.Hour &&
        eventStarttm.Minute == currentTime_.Minute && currentTime_.Hour >= 6 &&
        currentTime_.Hour < 22) {
      watchy_->vibrate(75, 5);
    }

    if (eventEnd - eventStart < SMALLEST_EVENT) {
      eventEnd = eventStart + SMALLEST_EVENT;
    }

    if (eventStart < windowStart) {
      eventStart = windowStart;
    } else {
      display->drawFastHLine(
          x0, y0 + ((eventStart - windowStart) / SECONDS_PER_PIXEL),
          targetWidth, color_);
    }
    if (eventEnd > windowEnd) {
      eventEnd = windowEnd;
    } else {
      display->drawFastHLine(
          x0, y0 + ((eventEnd - windowStart) / SECONDS_PER_PIXEL), targetWidth,
          color_);
    }
    int16_t eventOffset = (eventStart - windowStart) / SECONDS_PER_PIXEL;
    int16_t eventSize   = (eventEnd - eventStart) / SECONDS_PER_PIXEL;
    display->drawFastVLine(x0, y0 + eventOffset, eventSize, color_);
    display->drawFastVLine(x0 + targetWidth - 1, y0 + eventOffset, eventSize,
                           color_);
    if (targetWidth <= EVENT_PADDING * 2 || eventSize <= EVENT_PADDING * 2) {
      continue;
    }
    int16_t x1, y1;
    uint16_t tw, th;
    display->getTextBounds(event->summary, 0, 0, &x1, &y1, &tw, &th);
    if (tw + (EVENT_PADDING * 2) > targetWidth ||
        th + (EVENT_PADDING * 2) > eventSize) {
      resizeText(display, event->summary, MAX_EVENT_NAME_LEN,
                 targetWidth - (EVENT_PADDING * 2),
                 eventSize - (EVENT_PADDING * 2), &x1, &y1, &tw, &th);
    }

    if (th + (EVENT_PADDING * 2) <= eventSize) {
      display->setCursor(x0 - x1 + EVENT_PADDING,
                         y0 - y1 + eventOffset + EVENT_PADDING);
      display->print(event->summary);
    }
  }
}

void CalendarColumn::resizeText(Display *display, char *text, uint8_t buflen,
                                uint16_t width, uint16_t height, int16_t *x1,
                                int16_t *y1, uint16_t *tw, uint16_t *th) {
  String original = text;
  String result;
  int i = 0;
  while (true) {
    if (i >= original.length()) {
      break;
    }
    display->getTextBounds(result + original[i], 0, 0, x1, y1, tw, th);
    if (*tw > width) {
      // TODO: newlines aren't handled by the display library
      // in a way where the x offset is reset to the offset of
      // the previous line. instead, the x offset is reset to
      // the left of the screen! that means this approach won't
      // work here, and we will need to have the caller of
      // resize text find newlines and reset the cursor x
      // position or something. for now, just disable
      // subsequent lines.
      break;
    }
    result += original[i];
    i++;
  }
  result.toCharArray(text, buflen);
  display->getTextBounds(result, 0, 0, x1, y1, tw, th);
}

void CalendarHourBar::maybeDraw(Display *display, int16_t x0, int16_t y0,
                                uint16_t targetWidth, uint16_t targetHeight,
                                uint16_t *width, uint16_t *height, bool noop) {
  if (targetHeight <= 0) {
    targetHeight = 1;
  }
  *width  = 0;
  *height = targetHeight;

  display->setFont(SMALL_FONT);
  display->setTextColor(color_);

  time_t now         = unixEpochTime(currentTime_);
  time_t windowStart = now - CALENDAR_PAST_SECONDS;
  time_t windowEnd   = windowStart + (targetHeight * SECONDS_PER_PIXEL);

  tmElements_t currentHour = currentTime_;
  currentHour.Minute       = 0;
  currentHour.Second       = 0;
  time_t currentHourUnix   = unixEpochTime(currentHour);
  int currentHourNum       = currentHour.Hour;

  int16_t futureHours = (windowEnd - currentHourUnix) / 3600;

  for (int i = -(CALENDAR_PAST_SECONDS / 3600) - 1; i <= futureHours; i++) {
    time_t hourTime = currentHourUnix + (i * 60 * 60);
    if (hourTime + (3 * SECONDS_PER_PIXEL) < windowStart) {
      continue;
    }
    if (hourTime >= windowEnd) {
      continue;
    }

    String text(((currentHourNum + i + 11) % 12) + 1);
    int16_t x1, y1;
    uint16_t tw, th;
    display->getTextBounds(text, 0, 0, &x1, &y1, &tw, &th);
    if ((th + 3) * SECONDS_PER_PIXEL + hourTime > windowEnd) {
      continue;
    }
    if (tw + 4 > *width) {
      *width = tw + 4;
    }

    if (!noop) {
      int16_t yoffset = y0 + ((hourTime - windowStart) / SECONDS_PER_PIXEL);
      display->setCursor(x0 - x1 + 2, yoffset - y1 + 3);
      display->print(text);
    }
  }

  if (!noop) {
    display->drawFastHLine(x0, y0 + ((now - windowStart) / SECONDS_PER_PIXEL),
                           *width, color_);

    for (int i = -(CALENDAR_PAST_SECONDS / 3600) - 1; i <= futureHours; i++) {
      time_t hourTime = currentHourUnix + (i * 60 * 60);
      if (hourTime < windowStart) {
        continue;
      }
      if (hourTime >= windowEnd) {
        continue;
      }

      int16_t yoffset = y0 + ((hourTime - windowStart) / SECONDS_PER_PIXEL);
      display->drawFastHLine(x0, yoffset, *width / 2, color_);
    }
  }
}

void CalendarAlarms::maybeDraw(Display *display, int16_t x0, int16_t y0,
                               uint16_t targetWidth, uint16_t targetHeight,
                               uint16_t *width, uint16_t *height, bool noop) {
  *width                   = targetWidth;
  *height                  = 0;
  tmElements_t currentTime = watchy_->time();

  display->setFont(&FreeMono9pt7b);
  display->setTextColor(color_);

  time_t now         = unixEpochTime(currentTime);
  time_t windowStart = now - (2 * 60);
  time_t windowEnd   = now + (60 * 60);
  for (int i = 0; i < data_->alarmCount; i++) {
    alarmData *alarm = &(data_->alarms[i]);
    if (alarm->start < windowStart) {
      continue;
    }
    if (alarm->start > windowEnd) {
      continue;
    }
    tmElements_t alarmtm;
    fromUnixEpochTime(alarm->start, &alarmtm);
    if (!noop && alarmtm.Minute == currentTime.Minute &&
        alarmtm.Hour == currentTime.Hour) {
      watchy_->vibrate(100, 10);
    }
    String text;
    int hourNum = ((alarmtm.Hour + 11) % 12) + 1;
    if (hourNum < 10) {
      text += "0";
    }
    text += hourNum;
    text += (alarmtm.Minute < 10 ? ":0" : ":");
    text += alarmtm.Minute;
    text += ": ";
    text += alarm->summary;

    int16_t x1, y1;
    uint16_t tw, th;
    display->getTextBounds(text, 0, 0, &x1, &y1, &tw, &th);

    if (!noop) {
      display->setCursor(x0 - x1 + EVENT_PADDING,
                         y0 + *height - y1 + EVENT_PADDING);
      display->print(text);
    }

    *height += th + EVENT_PADDING;
  }
  if (*height > 0) {
    *height += EVENT_PADDING;
  }
}
