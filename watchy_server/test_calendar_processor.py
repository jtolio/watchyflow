#!/usr/bin/env python3
"""
Tests for the calendar processor.
"""

import datetime
import unittest
from unittest.mock import MagicMock, patch

import icalendar
from pytz import timezone

from main import CalendarProcessor, TIMEZONE


class TestCalendarProcessor(unittest.TestCase):
    """Tests for the CalendarProcessor class."""

    def setUp(self):
        """Set up test fixtures."""
        self.processor = CalendarProcessor(
            user_emails=["user@example.com", "user2@example.com"]
        )

    def test_has_required_fields(self):
        """Test has_required_fields method."""
        # Event with all required fields
        event = MagicMock()
        event.__contains__ = lambda self, key: key in ["DTSTART", "DTEND", "SUMMARY"]
        self.assertTrue(self.processor.has_required_fields(event))

        # Event missing a required field
        event.__contains__ = lambda self, key: key in ["DTSTART", "SUMMARY"]
        self.assertFalse(self.processor.has_required_fields(event))

    def create_mock_attendee(self, email, partstat=None):
        """Create a mock attendee."""
        attendee = MagicMock()
        attendee.params = {}
        if partstat:
            attendee.params["PARTSTAT"] = partstat
        # Mock the string representation to be the email
        attendee.__str__ = lambda self: f"mailto:{email}"
        return attendee

    def test_is_event_declined_by_user_no_attendees(self):
        """Test is_event_declined_by_user with no attendees."""
        event = {}
        self.assertFalse(self.processor.is_event_declined_by_user(event))

    def test_is_event_declined_by_user_not_declined(self):
        """Test is_event_declined_by_user with attendees but not declined."""
        event = {"ATTENDEE": self.create_mock_attendee("user@example.com", "ACCEPTED")}
        self.assertFalse(self.processor.is_event_declined_by_user(event))

    def test_is_event_declined_by_user_declined(self):
        """Test is_event_declined_by_user with a declined attendee."""
        event = {"ATTENDEE": self.create_mock_attendee("user@example.com", "DECLINED")}
        self.assertTrue(self.processor.is_event_declined_by_user(event))

    def test_is_event_declined_by_user_declined_one_of_many(self):
        """Test is_event_declined_by_user with multiple attendees."""
        event = {
            "ATTENDEE": [
                self.create_mock_attendee("other@example.com", "ACCEPTED"),
                self.create_mock_attendee("user@example.com", "DECLINED"),
                self.create_mock_attendee("third@example.com", "ACCEPTED"),
            ]
        }
        self.assertTrue(self.processor.is_event_declined_by_user(event))

    def test_is_event_declined_by_user_case_insensitive(self):
        """Test is_event_declined_by_user with case-insensitive email comparison."""
        event = {"ATTENDEE": self.create_mock_attendee("USER@example.com", "DECLINED")}
        self.assertTrue(self.processor.is_event_declined_by_user(event))

    def test_is_event_declined_by_user_not_user(self):
        """Test is_event_declined_by_user with a declined attendee that is not the user."""
        event = {"ATTENDEE": self.create_mock_attendee("other@example.com", "DECLINED")}
        self.assertFalse(self.processor.is_event_declined_by_user(event))

    def test_event_to_dict(self):
        """Test event_to_dict method."""
        # Create a mock event
        now = datetime.datetime.now(TIMEZONE)
        end = now + datetime.timedelta(hours=1)

        dt_start = MagicMock()
        dt_start.dt = now

        dt_end = MagicMock()
        dt_end.dt = end

        event = {
            "DTSTART": dt_start,
            "DTEND": dt_end,
            "SUMMARY": "Test Event",
            "STATUS": "CONFIRMED",
        }

        result = self.processor.event_to_dict(event)

        self.assertEqual(result["start"], int(now.timestamp()))
        self.assertEqual(result["end"], int(end.timestamp()))
        self.assertEqual(result["summary"], "Test Event")

    @patch("main.recurring_ical_events")
    @patch.object(CalendarProcessor, "fetch_calendar")
    def test_get_events(self, mock_fetch_calendar, mock_recurring_events):
        """Test get_events method."""
        # Create mock calendar and events
        mock_calendar = MagicMock()
        mock_events_object = MagicMock()
        mock_recurring_events.of.return_value = mock_events_object

        # Create some mock events
        now = datetime.datetime.now(TIMEZONE)
        dt_start = MagicMock()
        dt_start.dt = now

        dt_end = MagicMock()
        dt_end.dt = now + datetime.timedelta(hours=1)

        # Event 1: Has all required fields and not declined
        event1 = {"DTSTART": dt_start, "DTEND": dt_end, "SUMMARY": "Event 1"}

        # Event 2: Missing required field
        event2 = {"DTSTART": dt_start, "SUMMARY": "Event 2"}

        # Event 3: Declined by user
        event3 = {
            "DTSTART": dt_start,
            "DTEND": dt_end,
            "SUMMARY": "Event 3",
            "ATTENDEE": self.create_mock_attendee("user@example.com", "DECLINED"),
        }

        # Event 4: Has all required fields and not declined
        event4 = {"DTSTART": dt_start, "DTEND": dt_end, "SUMMARY": "Event 4"}

        # Set up mocks
        mock_fetch_calendar.return_value = mock_calendar
        mock_events_object.between.return_value = [event1, event2, event3, event4]

        # Call the method
        start_time = datetime.datetime.now()
        result, _ = self.processor.get_events(
            ["http://example.com/calendar.ics"], start_time
        )

        # Verify the results
        self.assertEqual(len(result), 2)  # Only event1 and event4 should be included
        self.assertEqual(result[0]["summary"], "Event 1")
        self.assertEqual(result[1]["summary"], "Event 4")

        # Verify mocks were called correctly
        mock_fetch_calendar.assert_called_once_with("http://example.com/calendar.ics", force_cache_miss=False)
        mock_recurring_events.of.assert_called_once_with(mock_calendar)
        mock_events_object.between.assert_called_once()


if __name__ == "__main__":
    unittest.main()
