#!/usr/bin/env python3
"""
Calendar helper server for embedded devices.

This server processes iCalendar files and returns upcoming events,
filtering out events that the user has declined.

Usage:
  python main.py --emails user@example.com another@example.com
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
    """Calendar processing logic, separated for testability."""

    def __init__(self, user_emails=None):
        """Initialize the calendar processor.

        Args:
            user_emails: List of email addresses that identify the user
        """
        self.user_emails = [email.lower() for email in (user_emails or [])]

    def fetch_calendar(self, url):
        """Fetch a calendar from a URL.

        Args:
            url: URL to fetch the calendar from

        Returns:
            Calendar object
        """
        resp = requests.get(url)
        resp.raise_for_status()
        return icalendar.Calendar.from_ical(resp.content)

    def is_event_declined_by_user(self, event):
        """Check if the event has been declined by the user.

        Args:
            event: Event object

        Returns:
            True if the event has been declined by the user, False otherwise
        """
        if "ATTENDEE" not in event or not self.user_emails:
            return False

        attendees = event["ATTENDEE"]
        # If single attendee, convert to list for consistent processing
        if not isinstance(attendees, list):
            attendees = [attendees]

        # Check each attendee
        for attendee in attendees:
            attendee_params = attendee.params
            # Only check PARTSTAT if it's set to DECLINED
            if (
                "PARTSTAT" in attendee_params
                and attendee_params["PARTSTAT"] == "DECLINED"
            ):
                # Get attendee email by removing mailto: prefix
                attendee_email = str(attendee).lower().replace("mailto:", "")

                # Check if this declined attendee is one of the user's identities
                for user_email in self.user_emails:
                    if user_email == attendee_email:
                        return True

        return False

    def convert_time(self, dt):
        if isinstance(dt, datetime.date):
                return dt.isoformat()
        return dt.astimezone(TIMEZONE).isoformat()

    def event_to_dict(self, event):
        """Convert an event to a dictionary.

        Args:
            event: Event object

        Returns:
            Dictionary representation of the event
        """
        return {
            "start": self.convert_time(event["DTSTART"].dt),
            "end": self.convert_time(event["DTEND"].dt),
            "summary": event["SUMMARY"],
        }

    def has_required_fields(self, event):
        """Check if an event has all required fields.

        Args:
            event: Event object

        Returns:
            True if the event has all required fields, False otherwise
        """
        for required in ("DTSTART", "DTEND", "SUMMARY"):
            if required not in event:
                return False
        return True

    def get_events(self, calendar_urls, start_time, end_time=None):
        """Get events from a list of calendar URLs.

        Args:
            calendar_urls: List of calendar URLs
            start_time: Start time for events
            end_time: End time for events (default: start_time + 7 hours)

        Returns:
            List of events sorted by start time, end time, and summary
        """
        if end_time is None:
            end_time = start_time + datetime.timedelta(hours=7)

        all_events = []

        for url in calendar_urls:
            try:
                calendar = self.fetch_calendar(url)
                events = recurring_ical_events.of(calendar).between(
                    start_time, end_time
                )

                for event in events:
                    # Skip events that don't have required fields
                    if not self.has_required_fields(event):
                        continue

                    # Skip events that have been declined by the user
                    if self.is_event_declined_by_user(event):
                        logging.debug(
                            f"Skipping declined event: {event.get('SUMMARY', 'No summary')}"
                        )
                        continue

                    if "STATUS" in event and event["STATUS"] != "CONFIRMED":
                        continue

                    # Add event to list
                    all_events.append(self.event_to_dict(event))
            except Exception as e:
                logging.error(f"Error fetching calendar {url}: {e}")

        # Sort events by start time, end time, and summary
        all_events.sort(
            key=lambda event: (event["start"], event["end"], event["summary"])
        )

        return all_events


class CalHandler(BaseHTTPRequestHandler):
    """HTTP handler for calendar requests."""

    def do_GET(self):
        """Handle GET requests."""
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

        # Get events from calendar processor
        start = when - datetime.timedelta(hours=1)
        processor = CalendarProcessor(
            user_emails=getattr(self.server, "user_emails", [])
        )
        all_events = processor.get_events(self.server.cals[key], start)

        # Send response
        self.send_response(200)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Encoding", "utf-8")
        self.end_headers()
        self.wfile.write(
            json.dumps(
                {
                    "status": "ok",
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

    # Split address into host and port
    host, port = args.addr.split(":")
    host = host if host else "0.0.0.0"
    port = int(port)

    # Create HTTP server
    server = HTTPServer((host, port), CalHandler)
    with open(args.cals, "rb") as fh:
        server.cals = json.load(fh)

    # Store user emails in server object to access in handler
    server.user_emails = [email.lower() for email in args.emails]
    logging.info(
        f"Filtering events for email identities: {', '.join(server.user_emails)}"
    )

    try:
        logging.info(f"Starting server on {host}:{port}")
        server.serve_forever()
    except KeyboardInterrupt:
        server.server_close()
        logging.info("Server stopped")


if __name__ == "__main__":
    main()
