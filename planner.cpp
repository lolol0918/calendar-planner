#include <ctime>
#include <iomanip>
#include <iostream>
#include <string>
using namespace std;

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

struct Calendar {
  int days[6][7];
  int month;
  int year;
};

bool isLeapYear(int year) {
  return (year % 400 == 0) or (year % 4 == 0 and year % 100 != 0);
}

int monthDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

int daysInMonth(int month, int year) {
  if (month < 1 || month > 12) {
    return 0;
  }
  if (month == 2 && isLeapYear(year)) {
    return 29;
  }

  return monthDays[month - 1];
}

// kung monday, tuesday, and so on gng.
int getStartingDay(int month, int year) {
  std::tm date = {};

  date.tm_year = year - 1900;
  date.tm_mon = month - 1;
  date.tm_mday = 1;
  date.tm_hour = 12;

  std::mktime(&date);

  return date.tm_wday;
}

// GLOBAL variables
Calendar calendar;
const int MAX_EVENTS = 100;

Event events[MAX_EVENTS];
int eventCount = 0;

// Stack for deleted events
Event deletedEvents[MAX_EVENTS];
int stackTop = -1;

// Queue for upcoming events
Event upcomingEvents[MAX_EVENTS];
int queueFront = 0;
int queueRear = -1;
int queueCount = 0;

void generateCalendar() {
  int startingDay = getStartingDay(calendar.month, calendar.year);
  int totalDays = daysInMonth(calendar.month, calendar.year);

  // Clear the calendar first
  for (int row = 0; row < 6; row++) {
    for (int col = 0; col < 7; col++) {
      calendar.days[row][col] = 0;
    }
  }

  // Fill in the day numbers
  for (int day = 1; day <= totalDays; day++) {
    int position = startingDay + day - 1;

    int row = position / 7;
    int col = position % 7;

    calendar.days[row][col] = day;
  }
}

void displayCalendar() {
  cout << "\n       " << calendar.month << "/" << calendar.year << "\n\n";

  cout << " Sun Mon Tue Wed Thu Fri Sat\n";

  for (int row = 0; row < 6; row++) {
    for (int col = 0; col < 7; col++) {
      if (calendar.days[row][col] == 0) {
        cout << "    ";
      } else {
        cout << setw(4) << calendar.days[row][col];
      }
    }
    cout << endl;
  }
}

void changeMonth(int direction) {
  calendar.month += direction;

  if (calendar.month > 12) {
    calendar.month = 1;
    calendar.year++;
  }

  if (calendar.month < 1) {
    calendar.month = 12;
    calendar.year--;
  }

  generateCalendar();
}

void addEvent() {
  if (eventCount >= MAX_EVENTS) {
    cout << "Event storage is full!\n";
    return;
  }

  Event newEvent;

  cin.ignore(10000, '\n');

  cout << "\nEvent title: ";
  getline(cin, newEvent.title);

  cout << "Description: ";
  getline(cin, newEvent.description);

  cout << "Day: ";
  cin >> newEvent.day;

  cout << "Month: ";
  cin >> newEvent.month;

  cout << "Year: ";
  cin >> newEvent.year;

  if (newEvent.month < 1 || newEvent.month > 12 || newEvent.day < 1 ||
      newEvent.day > daysInMonth(newEvent.month, newEvent.year)) {
    cout << "Invalid date!\n";
    return;
  }

  cout << "Start hour (0-23): ";
  cin >> newEvent.startHour;

  cout << "Start minute (0-59): ";
  cin >> newEvent.startMinute;

  cout << "End hour (0-23): ";
  cin >> newEvent.endHour;

  cout << "End minute (0-59): ";
  cin >> newEvent.endMinute;

  events[eventCount] = newEvent;
  eventCount++;

  cout << "Event added successfully!\n";
}

void deleteEvent() {
  if (eventCount == 0) {
    cout << "No events to delete.\n";
    return;
  }

  for (int i = 0; i < eventCount; i++) {
    cout << i + 1 << ". " << events[i].title << " (" << events[i].day << "/"
         << events[i].month << "/" << events[i].year << ")\n";
  }

  int choice;

  cout << "Choose event to delete: ";
  cin >> choice;

  if (choice < 1 || choice > eventCount) {
    cout << "Invalid choice.\n";
    return;
  }

  if (stackTop == MAX_EVENTS - 1) {
    cout << "Undo stack is full. Cannot delete.\n";
    return;
  }

  deletedEvents[++stackTop] = events[choice - 1];

  for (int i = choice - 1; i < eventCount - 1; i++) {
    events[i] = events[i + 1];
  }

  eventCount--;

  cout << "Event deleted. You can undo this action.\n";
}

void undoDelete() {
  if (stackTop == -1) {
    cout << "Nothing to undo.\n";
    return;
  }

  if (eventCount == MAX_EVENTS) {
    cout << "Event storage is full.\n";
    return;
  }

  events[eventCount] = deletedEvents[stackTop];
  eventCount++;

  stackTop--;

  cout << "Last deleted event restored!\n";
}

void displayEvents() {
  if (eventCount == 0) {
    cout << "No events available.\n";
    return;
  }

  int day, month, year;

  cout << "Enter day: ";
  cin >> day;

  cout << "Enter month: ";
  cin >> month;

  cout << "Enter year: ";
  cin >> year;

  bool found = false;

  for (int i = 0; i < eventCount; i++) {
    if (events[i].day == day && events[i].month == month &&
        events[i].year == year) {

      cout << "\nTitle: " << events[i].title;
      cout << "\nDescription: " << events[i].description;
      cout << "\nStart: " << events[i].startHour << ":"
           << events[i].startMinute;
      cout << "\nEnd: " << events[i].endHour << ":" << events[i].endMinute
           << "\n";

      found = true;
    }
  }

  if (!found) {
    cout << "No events found for that date.\n";
  }
}

void enqueueUpcoming(Event event) {
  if (queueCount == MAX_EVENTS) {
    cout << "Upcoming events queue is full.\n";
    return;
  }

  queueRear = (queueRear + 1) % MAX_EVENTS;
  upcomingEvents[queueRear] = event;
  queueCount++;
}

void dequeueUpcoming() {
  if (queueCount == 0) {
    cout << "No upcoming events.\n";
    return;
  }

  cout << "Next event: " << upcomingEvents[queueFront].title << "\n";

  queueFront = (queueFront + 1) % MAX_EVENTS;
  queueCount--;
}

void displayUpcoming() {
  if (queueCount == 0) {
    cout << "No upcoming events.\n";
    return;
  }

  cout << "\nUpcoming Events:\n";

  for (int i = 0; i < queueCount; i++) {
    int index = (queueFront + i) % MAX_EVENTS;

    cout << upcomingEvents[index].title << "\n";
  }
}

int main() {
  calendar.month = 12;
  calendar.year = 2026;

  generateCalendar();

  int choice;

  do {
    cout << "\n==============================\n";
    cout << "       CALENDAR PLANNER       \n";
    cout << "==============================\n";
    cout << "1. Display Calendar\n";
    cout << "2. Previous Month\n";
    cout << "3. Next Month\n";
    cout << "4. Add Event\n";
    cout << "5. View Events\n";
    cout << "6. Delete Event\n";
    cout << "7. Undo Delete\n";
    cout << "8. View Upcoming Events\n";
    cout << "9. Process Next Upcoming Event\n";
    cout << "10. Exit\n";
    cout << "==============================\n";
    cout << "Choose: ";

    if (!(cin >> choice)) {
      cin.clear();
      cin.ignore(10000, '\n');
      cout << "Invalid input.\n";
      continue;
    }

    switch (choice) {
    case 1:
      displayCalendar();
      break;
    case 2:
      changeMonth(-1);
      displayCalendar();
      break;
    case 3:
      changeMonth(1);
      displayCalendar();
      break;
    case 4:
      addEvent();
      break;
    case 5:
      displayEvents();
      break;
    case 6:
      deleteEvent();
      break;
    case 7:
      undoDelete();
      break;
    case 8:
      displayUpcoming();
      break;
    case 9:
      dequeueUpcoming();
      break;
    case 10:
      cout << "Goodbye!\n";
      break;
    default:
      cout << "Invalid choice.\n";
    }

  } while (choice != 10);

  return 0;
}
