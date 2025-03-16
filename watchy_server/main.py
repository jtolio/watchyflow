#!/usr/bin/env python3
"""
Calendar helper server for embedded devices.

This server processes iCalendar files and returns upcoming events.
"""

import argparse
import datetime
import json
import logging
import urllib.parse
from http.server import HTTPServer, BaseHTTPRequestHandler

import dateutil.parser
import icalendar
from pytz import timezone
import recurring_ical_events
import requests

TIMEZONE = timezone("US/Eastern")


class CalendarProcessor:

    def __init__(self, user_emails=None):
        self.user_emails = [email.lower() for email in (user_emails or [])]

    def fetch_calendar(self, url):
        resp = requests.get(url)
        resp.raise_for_status()
        return icalendar.Calendar.from_ical(resp.content)

    def is_event_declined_by_user(self, event):
        if "ATTENDEE" not in event or not self.user_emails:
            return False

        attendees = event["ATTENDEE"]
        # If single attendee, convert to list for consistent processing
        if not isinstance(attendees, list):
            attendees = [attendees]

        for attendee in attendees:
            attendee_params = attendee.params
            if (
                "PARTSTAT" in attendee_params
                and attendee_params["PARTSTAT"] == "DECLINED"
            ):
                attendee_email = str(attendee).lower().replace("mailto:", "")

                for user_email in self.user_emails:
                    if user_email == attendee_email:
                        return True

        return False

    def convert_time(self, dt):
        if hasattr(dt, "astimezone"):
            dt = dt.astimezone(TIMEZONE)
        return dt.isoformat()

    def event_to_dict(self, event):
        start = event["DTSTART"].dt
        end = event["DTEND"].dt
        rv = {
            "start": self.convert_time(start),
            "end": self.convert_time(end),
            "summary": event["SUMMARY"],
            "day": False,
        }
        if not hasattr(start, "astimezone") or not hasattr(end, "astimezone"):
            rv["day"] = True
        else:
            rv["start-unix"] = start.timestamp()
            rv["end-unix"] = end.timestamp()
        return rv

    def has_required_fields(self, event):
        for required in ("DTSTART", "DTEND", "SUMMARY"):
            if required not in event:
                return False
        return True

    def get_events(self, calendar_urls, start_time, end_time=None):
        if end_time is None:
            end_time = start_time + datetime.timedelta(hours=7)

        all_events = []
        event_edges = []

        for url in calendar_urls:
            try:
                calendar = self.fetch_calendar(url)
                events = recurring_ical_events.of(calendar).between(
                    start_time, end_time
                )

                for event in events:
                    if not self.has_required_fields(event):
                        continue

                    if self.is_event_declined_by_user(event):
                        continue

                    if "STATUS" in event and event["STATUS"] != "CONFIRMED":
                        continue

                    event = self.event_to_dict(event)

                    if event["end"] <= event["start"]:
                        continue

                    event_id = len(all_events)
                    all_events.append(event)
                    event_edges.append((event["start"], "1", event["end"], event_id))
                    event_edges.append((event["end"], "0", "", event_id))
            except Exception as e:
                logging.error(f"Error fetching calendar {url}: {e}")

        # python documentation crazily recommends that if you want to sort
        # by multiple fields in different directions, say one field ascending
        # and another descending, you should rely on python's stable sorting
        # guarantee and timsort's already-sorted-data-speed to just sort
        # the data twice. this feels crazy, but that's what we'll do I guess.
        # we want the data sorted by the first field and second field ascending,
        # followed by the third field descending, so the way to do this is to
        # first sort by the third field descending, then the first two.
        # the reason we want the data sorted this way is that for a given
        # specific colliding timestamp, we want event end edges to come first,
        # followed by event starts for events that are longer, followed by
        # event starts that are shorter. otherwise, event edges should be in
        # timestamp order.
        event_edges.sort(key=lambda x: x[2], reverse=True)
        event_edges.sort(key=lambda x: (x[0], x[1]))

        free_columns = []
        next_column = 0
        for timestamp, is_start, _, event_id in event_edges:
            is_start = is_start == "1"
            if not is_start:
                column = all_events[event_id]["column"]
                if column >= 0:
                    free_columns.append(column)
                continue
            if len(timestamp) == len("0000-00-00"):
                all_events[event_id]["column"] = -1
                continue
            if free_columns:
                free_columns.sort()
                all_events[event_id]["column"] = free_columns.pop(0)
                continue
            all_events[event_id]["column"] = next_column
            next_column += 1
            continue

        all_events.sort(
            key=lambda event: (event["start"], event["end"], event["column"], event["summary"])
        )

        return all_events, next_column


class CalHandler(BaseHTTPRequestHandler):

    def do_GET(self):
        url = urllib.parse.urlparse(self.path)
        query = urllib.parse.parse_qs(url.query)
        prefix = "/v0/account/"
        if not url.path.startswith(prefix):
            self.send_response(404)
            self.end_headers()
            return

        key = url.path[len(prefix) :]
        if key not in self.server.cals:
            self.send_response(404)
            self.end_headers()
            return

        when = (query.get("time") or ["now"])[-1]
        if when == "now":
            when = datetime.datetime.now()
        else:
            when = dateutil.parser.parse(when)

        start = when - datetime.timedelta(hours=1)
        processor = CalendarProcessor(
            user_emails=getattr(self.server, "user_emails", [])
        )
        all_events, columns = processor.get_events(self.server.cals[key], start)

        self.send_response(200)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Encoding", "utf-8")
        self.end_headers()
        self.wfile.write(
            json.dumps(
                {
                    "status": "ok",
                    "columns": columns,
                    "events": all_events,
                }
            ).encode("utf8")
        )


def main():
    logging.basicConfig(format="%(message)s", level=logging.INFO)
    parser = argparse.ArgumentParser()
    parser.add_argument("--addr", default=":8081", help="listen address (host:port)")
    parser.add_argument(
        "--cals", default="cals.json", help="configuration file for calendars"
    )
    parser.add_argument(
        "--emails",
        nargs="+",
        default=[],
        help="Your email addresses to identify yourself in calendar events (for filtering declined events)",
    )
    args = parser.parse_args()

    host, port = args.addr.split(":")
    host = host if host else "0.0.0.0"
    port = int(port)

    server = HTTPServer((host, port), CalHandler)
    with open(args.cals, "rb") as fh:
        server.cals = json.load(fh)

    server.user_emails = [email.lower() for email in args.emails]

    try:
        logging.info(f"Starting server on {host}:{port}")
        server.serve_forever()
    except KeyboardInterrupt:
        server.server_close()
        logging.info("Server stopped")


if __name__ == "__main__":
    main()
