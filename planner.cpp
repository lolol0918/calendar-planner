#include <iomanip>
#include <iostream>
#include <queue>
#include <sstream>
#include <stack>
#include <string>
using namespace std;

const int MAX_EVENTS = 100;
const int MIN_YEAR = 1;
const int MAX_YEAR = 9999;

// ANSI escape code for clearing screen
void clearScreen() { cout << "\033[H\033[2J\033[3J" << flush; }

void pauseScreen() {
  cout << "\n\nPress Enter to continue...";
  cin.get();
}
// One event on the planner.
struct Event {
  string title;
  string description;
  int day;
  int month;
  int year;
  int startHour;
  int startMinute;
  int endHour;
  int endMinute;
};

// The month currently shown on screen.
struct Calendar {
  int days[6][7];
  int month;
  int year;
  int startingDay; // weekday of the 1st: 0 = Sunday ... 6 = Saturday
};

// Returns true if the given year is a leap year.
bool isLeapYear(int year) {
  return (year % 400 == 0) || (year % 4 == 0 && year % 100 != 0);
}

const int monthDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

const string monthNames[] = {"January",   "February", "March",    "April",
                             "May",       "June",     "July",     "August",
                             "September", "October",  "November", "December"};

// Returns how many days a month has, accounting for leap years.
int daysInMonth(int month, int year) {
  if (month < 1 || month > 12) {
    return 0;
  }
  if (month == 2 && isLeapYear(year)) {
    return 29;
  }

  return monthDays[month - 1];
}

// Returns the name of a month, e.g. 12 -> "December".
string monthName(int month) {
  if (month < 1 || month > 12) {
    return "Unknown";
  }

  return monthNames[month - 1];
}

int getStartingDay(int month, int year) {
  int monthOffset[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};

  if (month < 3) {
    year--;
  }

  int day = year + year / 4 - year / 100 + year / 400;
  day += monthOffset[month - 1] + 1;

  return day % 7;
}

// GLOBAL variables
Calendar calendar;

Event events[MAX_EVENTS];
int eventCount = 0;

stack<Event> deletedEvents;
queue<Event> upcomingEvents;

// Converts a date to text, e.g. "December 5, 2026".
string formatDate(int day, int month, int year) {
  return monthName(month) + " " + to_string(day) + ", " + to_string(year);
}

// Same as formatDate, but takes the date from an event.
string formatEventDate(const Event &event) {
  return formatDate(event.day, event.month, event.year);
}

// Converts 24-hour time to text, e.g. (14, 5) -> "2:05 PM".
string formatTime(int hour, int minute) {
  string period;

  if (hour < 12) {
    period = "AM";
  } else {
    period = "PM";
  }

  int displayHour = hour % 12;

  if (displayHour == 0) {
    displayHour = 12;
  }

  string minuteText = to_string(minute);

  if (minute < 10) {
    minuteText = "0" + minuteText;
  }

  return to_string(displayHour) + ":" + minuteText + " " + period;
}

string formatTimeRange(const Event &event) {
  return formatTime(event.startHour, event.startMinute) + " - " +
         formatTime(event.endHour, event.endMinute);
}

// Converts a time to minutes after midnight, so two times can be compared.
int toMinutes(int hour, int minute) { return hour * 60 + minute; }

// Returns a number used to sort events by date and starting time.
long long eventKey(const Event &event) {
  long long date = event.year;

  date = date * 100 + event.month;
  date = date * 100 + event.day;

  return date * 10000 + toMinutes(event.startHour, event.startMinute);
}

// Returns true if both events have the same title, date, and start time.
bool isSameEvent(const Event &first, const Event &second) {
  return first.title == second.title && eventKey(first) == eventKey(second);
}

// Returns true if the text is empty or contains only spaces.

bool isBlank(const string &text) {
  for (char c : text) {
    if (c != ' ' && c != '\t') {
      return false;
    }
  }

  return true;
}

// ----------------------------------------------------------------------------
// Input
// ----------------------------------------------------------------------------

// Reads one line of input. Exits cleanly if the input stream is closed,
// which would otherwise cause an endless loop in the input functions.
string readLine(string prompt) {
  cout << prompt;

  string input;
  getline(cin, input);

  return input;
}

// Keeps asking until the user enters a whole number between min and max.
int readInt(const string &prompt, int minValue, int maxValue) {
  while (true) {
    stringstream stream(readLine(prompt));
    int value;
    char extra;

    if ((stream >> value) && !(stream >> extra) && value >= minValue &&
        value <= maxValue) {
      return value;
    }

    cout << "Please enter a number from " << minValue << " to " << maxValue
         << ".\n";
  }
}

// Keeps asking until the day, month, and year form a real date.
void readDate(int &day, int &month, int &year) {
  while (true) {
    day = readInt("Day (1-31): ", 1, 31);
    month = readInt("Month (1-12): ", 1, 12);
    year = readInt("Year: ", MIN_YEAR, MAX_YEAR);

    int totalDays = daysInMonth(month, year);

    if (day <= totalDays) {
      return;
    }

    cout << monthName(month) << " " << year << " only has " << totalDays
         << " days. Please try again.\n";
  }
}

// ----------------------------------------------------------------------------
// Calendar
// ----------------------------------------------------------------------------

// Fills the 2D array with the day numbers of the current month.
void generateCalendar() {
  calendar.startingDay = getStartingDay(calendar.month, calendar.year);
  int totalDays = daysInMonth(calendar.month, calendar.year);

  // Clear the calendar first
  for (int row = 0; row < 6; row++) {
    for (int col = 0; col < 7; col++) {
      calendar.days[row][col] = 0;
    }
  }

  // Fill in the day numbers
  for (int day = 1; day <= totalDays; day++) {
    int position = calendar.startingDay + day - 1;

    int row = position / 7;
    int col = position % 7;

    calendar.days[row][col] = day;
  }
}

bool hasEvent(int day, int month, int year) {
  for (int i = 0; i < eventCount; i++) {
    if (events[i].day == day && events[i].month == month &&
        events[i].year == year) {
      return true;
    }
  }

  return false;
}

// Prints the current month as a calendar grid.
void displayCalendar() {
  string title = monthName(calendar.month) + " " + to_string(calendar.year);
  int calendarWidth = 28;
  int padding = (calendarWidth - static_cast<int>(title.length())) / 2;

  cout << "\n";
  cout << string(padding, ' ') << title << "\n\n";

  cout << " Sun Mon Tue Wed Thu Fri Sat\n";

  int totalDays = daysInMonth(calendar.month, calendar.year);
  int totalRows = (calendar.startingDay + totalDays + 6) / 7;

  for (int row = 0; row < totalRows; row++) {
    int lastCol = 6;

    while (lastCol >= 0 && calendar.days[row][lastCol] == 0) {
      lastCol--;
    }

    for (int col = 0; col <= lastCol; col++) {
      int day = calendar.days[row][col];

      if (day == 0) {
        cout << "    ";
      } else if (hasEvent(day, calendar.month, calendar.year)) {
        cout << setw(3) << "*" << day;
      } else {
        cout << setw(4) << day;
      }
    }

    cout << "\n";
  }
}

// Moves the calendar one month back (-1) or forward (+1).
// Stops at the first and last supported years.
void changeMonth(int direction) {
  int newMonth = calendar.month + direction;
  int newYear = calendar.year;

  if (newMonth > 12) {
    newMonth = 1;
    newYear++;
  } else if (newMonth < 1) {
    newMonth = 12;
    newYear--;
  }

  if (newYear < MIN_YEAR || newYear > MAX_YEAR) {
    cout << "Cannot go beyond the years " << MIN_YEAR << " to " << MAX_YEAR
         << ".\n";
    return;
  }

  calendar.month = newMonth;
  calendar.year = newYear;

  generateCalendar();
}

// ----------------------------------------------------------------------------
// Event list and upcoming queue
// ----------------------------------------------------------------------------

// Adds an event to the back of the upcoming queue.
void enqueueUpcoming(const Event &event) {
  if (upcomingEvents.size() == MAX_EVENTS) {
    cout << "Upcoming events queue is full.\n";
    return;
  }

  upcomingEvents.push(event);
}

// Refills the upcoming queue from events[]. Since events[] is always sorted,
// the queue ends up in chronological order. Called whenever events[] changes.
void rebuildUpcoming() {
  upcomingEvents = queue<Event>();

  for (int i = 0; i < eventCount; i++) {
    enqueueUpcoming(events[i]);
  }
}

// Inserts an event at its chronological position in events[].
// The caller must make sure the array is not full.
void insertSorted(const Event &event) {
  int position = eventCount;

  while (position > 0 && eventKey(events[position - 1]) > eventKey(event)) {
    events[position] = events[position - 1];
    position--;
  }

  events[position] = event;
  eventCount++;
}

// Removes the event at the given index and closes the gap.
void removeEventAt(int index) {
  for (int i = index; i < eventCount - 1; i++) {
    events[i] = events[i + 1];
  }

  eventCount--;
}

// ----------------------------------------------------------------------------
// Menu actions
// ----------------------------------------------------------------------------

void addEvent() {
  if (eventCount >= MAX_EVENTS) {
    cout << "Event storage is full!\n";
    return;
  }

  Event newEvent;

  cout << "\nAdd Event\n";
  cout << "---------\n";

  // A title is required
  while (true) {
    newEvent.title = readLine("Title: ");

    if (!isBlank(newEvent.title)) {
      break;
    }

    cout << "The title cannot be empty.\n";
  }

  newEvent.description = readLine("Description: ");

  readDate(newEvent.day, newEvent.month, newEvent.year);

  newEvent.startHour = readInt("Start hour (0-23): ", 0, 23);
  newEvent.startMinute = readInt("Start minute (0-59): ", 0, 59);

  // The event must end after it starts
  while (true) {
    newEvent.endHour = readInt("End hour (0-23): ", 0, 23);
    newEvent.endMinute = readInt("End minute (0-59): ", 0, 59);

    if (toMinutes(newEvent.endHour, newEvent.endMinute) >
        toMinutes(newEvent.startHour, newEvent.startMinute)) {
      break;
    }

    cout << "The end time must be later than the start time.\n";
  }

  insertSorted(newEvent);
  rebuildUpcoming();

  cout << "\nEvent added: " << newEvent.title << "\n";
  cout << "  " << formatEventDate(newEvent);
  cout << ", " << formatTimeRange(newEvent) << "\n";
}

void deleteEvent() {
  if (eventCount == 0) {
    cout << "No events to delete.\n";
    return;
  }

  if (deletedEvents.size() == MAX_EVENTS) {
    cout << "Undo stack is full. Cannot delete.\n";
    return;
  }

  cout << "\nDelete Event\n";
  cout << "------------\n";

  for (int i = 0; i < eventCount; i++) {
    cout << " " << i + 1 << ". " << events[i].title << " - "
         << formatEventDate(events[i]) << "\n";
  }

  int choice =
      readInt("\nChoose event to delete (0 to cancel): ", 0, eventCount);

  if (choice == 0) {
    cout << "Deletion cancelled.\n";
    return;
  }

  // Save the event on the stack so the deletion can be undone
  deletedEvents.push(events[choice - 1]);

  removeEventAt(choice - 1);
  rebuildUpcoming();

  cout << "\nDeleted \"" << deletedEvents.top().title << "\" ("
       << formatEventDate(deletedEvents.top()) << ").\n";
  cout << "You can undo this from the main menu.\n";
}

void undoDelete() {
  if (deletedEvents.empty()) {
    cout << "Nothing to undo.\n";
    return;
  }

  if (eventCount == MAX_EVENTS) {
    cout << "Event storage is full.\n";
    return;
  }

  // Pop the last deleted event and put it back in its chronological spot
  Event restored = deletedEvents.top();
  deletedEvents.pop();

  insertSorted(restored);
  rebuildUpcoming();

  cout << "Restored \"" << restored.title << "\" (" << formatEventDate(restored)
       << ").\n";
}

void displayEvents() {
  if (eventCount == 0) {
    cout << "No events available.\n";
    return;
  }

  int day, month, year;

  cout << "\nView Events by Date\n";
  cout << "-------------------\n";

  readDate(day, month, year);

  string heading = "Events on " + formatDate(day, month, year);
  bool found = false;

  for (int i = 0; i < eventCount; i++) {
    if (events[i].day == day && events[i].month == month &&
        events[i].year == year) {
      if (!found) {
        cout << "\n" << heading << "\n";
        cout << string(heading.length(), '-') << "\n";
      }

      cout << "\n" << events[i].title << "\n";

      if (!events[i].description.empty()) {
        cout << "  " << events[i].description << "\n";
      }

      cout << "  " << formatTimeRange(events[i]) << "\n";

      found = true;
    }
  }

  if (!found) {
    cout << "No events found for " << formatDate(day, month, year) << ".\n";
  }
}

// Takes the earliest event off the queue and marks it as completed.
// A completed event is also removed from the event list.
void dequeueUpcoming() {
  if (upcomingEvents.empty()) {
    cout << "No upcoming events.\n";
    return;
  }

  Event nextEvent = upcomingEvents.front();

  cout << "Next event: " << nextEvent.title << "\n";
  cout << "  " << formatEventDate(nextEvent);
  cout << ", " << formatTimeRange(nextEvent) << "\n";

  upcomingEvents.pop();

  for (int i = 0; i < eventCount; i++) {
    if (isSameEvent(events[i], nextEvent)) {
      removeEventAt(i);
      break;
    }
  }

  cout << "Event completed.\n";
}

void displayUpcoming() {
  if (upcomingEvents.empty()) {
    cout << "No upcoming events.\n";
    return;
  }

  cout << "\nUpcoming Events\n";
  cout << "---------------\n";

  queue<Event> remaining = upcomingEvents;

  for (int i = 0; !remaining.empty(); i++) {
    Event event = remaining.front();

    cout << " " << i + 1 << ". " << event.title << "\n";
    cout << "    " << formatEventDate(event) << ", " << formatTimeRange(event)
         << "\n";

    remaining.pop();
  }
}

void displayMenu() {
  cout << "\n";
  cout << "========================================================\n";
  cout << "                         <MENU>\n";
  cout << "========================================================\n";

  cout << "  [1] Previous Month         [5] Delete Event\n";
  cout << "  [2] Next Month             [6] Undo Delete\n";
  cout << "  [3] Add Event              [7] Upcoming Events\n";
  cout << "  [4] View Events by Date    [8] Complete Next Event\n";
  cout << "--------------------------------------------------------\n";
  cout << "  [9] Exit\n";
  cout << "========================================================\n";
}

int main() {
  calendar.month = 12;
  calendar.year = 2026;

  generateCalendar();

  int choice;

  do {
    clearScreen();

    displayCalendar();
    displayMenu();

    choice = readInt("Choice: ", 1, 9);

    switch (choice) {
    case 1:
      changeMonth(-1);
      break;
    case 2:
      changeMonth(1);
      break;
    case 3:
      addEvent();
      pauseScreen();
      break;
    case 4:
      displayEvents();
      pauseScreen();
      break;
    case 5:
      deleteEvent();
      pauseScreen();
      break;
    case 6:
      undoDelete();
      pauseScreen();
      break;
    case 7:
      displayUpcoming();
      pauseScreen();
      break;
    case 8:
      dequeueUpcoming();
      pauseScreen();
      break;
    case 9:
      cout << "THANK YOU FOR USING THE PROGRAM!\n";
      break;
    }

  } while (choice != 9);

  return 0;
}
