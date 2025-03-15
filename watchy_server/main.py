#!/usr/bin/env python3

import argparse
import datetime
import json
import logging
from http.server import HTTPServer, BaseHTTPRequestHandler

import icalendar
from pytz import timezone
import recurring_ical_events
import requests

TIMEZONE = timezone("US/Eastern")


class CalHandler(BaseHTTPRequestHandler):

    def do_GET(self):
        prefix = "/account/"
        if not self.path.startswith(prefix):
            self.send_response(404)
            self.end_headers()
            return

        key = self.path[len(prefix) :]
        if key not in self.server.cals:
            self.send_response(404)
            self.end_headers()
            return

        all_events = []
        start = datetime.datetime.now() - datetime.timedelta(hours=1)
        end = start + datetime.timedelta(hours=7)

        for cal_url in self.server.cals[key]:
            resp = requests.get(cal_url)
            resp.raise_for_status()
            for event in recurring_ical_events.of(
                    icalendar.Calendar.from_ical(resp.content)).between(
                    start, end):
                skip = False
                for required in ("DTSTART", "DTEND", "SUMMARY"):
                    if required not in event:
                        skip = True
                        break
                if skip: continue
                status = "CONFIRMED"
                if "status" in event:
                    status = event["STATUS"]
                all_events.append({
                        "start": event["DTSTART"].dt.astimezone(TIMEZONE).isoformat(),
                        "end": event["DTEND"].dt.astimezone(TIMEZONE).isoformat(),
                        "status": status,
                        "summary": event["SUMMARY"],
                    })

        self.send_response(200)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Encoding", "utf-8")
        self.end_headers()
        self.wfile.write(json.dumps({
            "status": "ok",
            "events": all_events,
        }).encode("utf8"))


def main():
    logging.basicConfig(format="%(message)s", level=logging.INFO)
    parser = argparse.ArgumentParser()
    parser.add_argument("--addr", default=":8081", help="listen address (host:port)")
    parser.add_argument(
        "--cals", default="cals.json", help="configuration file for calendars"
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

    try:
        logging.info(f"Starting server on {host}:{port}")
        server.serve_forever()
    except KeyboardInterrupt:
        server.server_close()
        logging.info("Server stopped")


if __name__ == "__main__":
    main()
