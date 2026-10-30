
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cctype>
using namespace std;

const int COACHES = 2;
const int SEATS = 10;

struct Passenger {
    int pnr;
    string name;
    int priority;
    char preference;
    bool strict;
    bool active;
    bool booked;
    int coach;
    int seat;
    int order;
};

struct Seat {
    int passengerIndex;
};

Seat seats[COACHES][SEATS];
vector<Passenger> passengers;

int nextPNR = 1001;
int bookingOrder = 0;
int hoursRemaining = 720;

// Priority 1 is highest. Lower berths are seats 1 to 5.
int getOccupancy(int coach) {
    int count = 0;

    for (int i = 0; i < SEATS; i++) {
        if (seats[coach][i].passengerIndex != -1)
            count++;
    }

    return count;
}

string getPriorityName(int priority) {
    if (priority == 1) return "Pregnant Woman";
    if (priority == 2) return "Senior Citizen";
    if (priority == 3) return "Person with Disability";
    if (priority == 4) return "Child";
    return "General Passenger";
}

string getBerthName(int seat) {
    if (seat < 5)
        return "Lower";
    return "Upper";
}

bool isLowerProtected(int coach) {
    return getOccupancy(coach) >= 5 && hoursRemaining > 120;
}

int findSeat(int coach, char type, int priority) {
    int start, end;

    if (type == 'L') {
        start = 0;
        end = 5;

        // General passengers cannot use protected Lower berths.
        if (isLowerProtected(coach) && priority == 5)
            return -1;
    } else {
        start = 5;
        end = 10;
    }

    for (int i = start; i < end; i++) {
        if (seats[coach][i].passengerIndex == -1)
            return i;
    }

    return -1;
}

bool allocateSeat(int index) {
    Passenger &p = passengers[index];

    for (int c = 0; c < COACHES; c++) {
        int seat = findSeat(c, p.preference, p.priority);

        if (seat == -1 && !p.strict) {
            char other = (p.preference == 'L') ? 'U' : 'L';
            seat = findSeat(c, other, p.priority);
        }

        if (seat != -1) {
            seats[c][seat].passengerIndex = index;

            p.booked = true;
            p.coach = c;
            p.seat = seat;

            cout << "\nBooking Confirmed!\n";
            cout << "PNR: " << p.pnr << "\n";
            cout << "Coach: " << c + 1 << "\n";
            cout << "Seat: " << seat + 1 << "\n";
            cout << "Berth: " << getBerthName(seat) << "\n";

            return true;
        }
    }

    return false;
}

void showSeats() {
    cout << "\n========== SEAT STATUS ==========\n";

    for (int c = 0; c < COACHES; c++) {
        cout << "\nCoach " << c + 1 << "\n";
        cout << "Occupancy: " << getOccupancy(c)
             << "/10\n";

        if (isLowerProtected(c))
            cout << "Lower Protection: ACTIVE\n";
        else
            cout << "Lower Protection: INACTIVE\n";

        for (int i = 0; i < SEATS; i++) {
            cout << (i + 1) << ". ";

            if (seats[c][i].passengerIndex == -1) {
                cout << getBerthName(i) << " - Available\n";
            } else {
                int index = seats[c][i].passengerIndex;

                cout << getBerthName(i) << " - Occupied"
                     << " (PNR " << passengers[index].pnr
                     << ")\n";
            }
        }
    }
}

void bookTicket() {
    Passenger p;

    cout << "\nEnter passenger name: ";
    getline(cin >> ws, p.name);

    cout << "\nPassenger Priority:\n";
    cout << "1. Pregnant Woman\n";
    cout << "2. Senior Citizen\n";
    cout << "3. Person with Disability\n";
    cout << "4. Child\n";
    cout << "5. General Passenger\n";
    cout << "Enter priority (1-5): ";
    cin >> p.priority;

    if (p.priority < 1 || p.priority > 5) {
        cout << "Invalid priority!\n";
        return;
    }

    cout << "Preferred berth (L = Lower, U = Upper): ";
    cin >> p.preference;
    p.preference = toupper(p.preference);

    if (p.preference != 'L' && p.preference != 'U') {
        cout << "Invalid berth preference!\n";
        return;
    }

    char choice;
    cout << "Strict preference? (Y/N): ";
    cin >> choice;
    choice = toupper(choice);

    if (choice != 'Y' && choice != 'N') {
        cout << "Invalid choice!\n";
        return;
    }

    p.pnr = nextPNR++;
    p.strict = (choice == 'Y');
    p.active = true;
    p.booked = false;
    p.coach = -1;
    p.seat = -1;
    p.order = bookingOrder++;

    passengers.push_back(p);
    int index = (int)passengers.size() - 1;

    if (!allocateSeat(index)) {
        cout << "\nNo suitable berth is available.\n";
        cout << "Passenger added to the waiting list.\n";
        cout << "Waiting PNR: " << p.pnr << "\n";
    }
}

void medicalUpgrade() {
    for (int c = 0; c < COACHES; c++) {
        int lowerSeat = findSeat(c, 'L', 1);

        if (lowerSeat == -1)
            continue;

        for (int i = 0; i < (int)passengers.size(); i++) {
            Passenger &p = passengers[i];

            if (p.active && p.booked &&
                p.priority == 1 &&
                p.preference == 'L' &&
                p.seat >= 5) {

                int oldCoach = p.coach;
                int oldSeat = p.seat;

                seats[oldCoach][oldSeat].passengerIndex = -1;
                seats[c][lowerSeat].passengerIndex = i;

                p.coach = c;
                p.seat = lowerSeat;

                cout << "\nMedical Upgrade!\n";
                cout << "PNR " << p.pnr
                     << " upgraded to Lower berth.\n";

                break;
            }
        }
    }
}

void processWaitingList() {
    vector<int> waiting;

    for (int i = 0; i < (int)passengers.size(); i++) {
        if (passengers[i].active && !passengers[i].booked)
            waiting.push_back(i);
    }

    // Higher priority first; same priority follows FCFS.
    sort(waiting.begin(), waiting.end(),
        [](int a, int b) {
            if (passengers[a].priority != passengers[b].priority)
                return passengers[a].priority < passengers[b].priority;

            return passengers[a].order < passengers[b].order;
        });

    for (int index : waiting) {
        if (allocateSeat(index))
            cout << "Waiting-list passenger allocated a berth.\n";
    }
}

void cancelTicket() {
    int pnr;

    cout << "\nEnter PNR to cancel: ";
    cin >> pnr;

    for (int i = 0; i < (int)passengers.size(); i++) {
        Passenger &p = passengers[i];

        if (p.pnr == pnr && p.active) {
            if (p.booked) {
                seats[p.coach][p.seat].passengerIndex = -1;
            }

            p.active = false;
            p.booked = false;

            cout << "Booking cancelled successfully.\n";

            // Priority-1 medical upgrade is checked first.
            medicalUpgrade();
            processWaitingList();
            return;
        }
    }

    cout << "Active PNR not found.\n";
}

void showBookings() {
    cout << "\n========== PASSENGER DETAILS ==========\n";

    bool found = false;

    for (const Passenger &p : passengers) {
        if (!p.active)
            continue;

        found = true;

        cout << "\nPNR: " << p.pnr << "\n";
        cout << "Name: " << p.name << "\n";
        cout << "Priority: " << getPriorityName(p.priority) << "\n";
        cout << "Preference: " << getBerthName(
            p.preference == 'L' ? 0 : 5) << "\n";
        cout << "Strict Preference: "
             << (p.strict ? "Yes" : "No") << "\n";

        if (p.booked) {
            cout << "Status: Confirmed\n";
            cout << "Coach: " << p.coach + 1 << "\n";
            cout << "Seat: " << p.seat + 1 << "\n";
            cout << "Allocated Berth: "
                 << getBerthName(p.seat) << "\n";
        } else {
            cout << "Status: Waiting List\n";
        }
    }

    if (!found)
        cout << "No active bookings.\n";
}

void showWaitingList() {
    vector<int> waiting;

    for (int i = 0; i < (int)passengers.size(); i++) {
        if (passengers[i].active && !passengers[i].booked)
            waiting.push_back(i);
    }

    sort(waiting.begin(), waiting.end(),
        [](int a, int b) {
            if (passengers[a].priority != passengers[b].priority)
                return passengers[a].priority < passengers[b].priority;

            return passengers[a].order < passengers[b].order;
        });

    cout << "\n========== WAITING LIST ==========\n";

    if (waiting.empty()) {
        cout << "Waiting list is empty.\n";
        return;
    }

    for (int index : waiting) {
        Passenger &p = passengers[index];

        cout << "PNR: " << p.pnr
             << " | Name: " << p.name
             << " | Priority: " << p.priority
             << " | Preference: " << p.preference
             << " | Strict: " << (p.strict ? "Yes" : "No")
             << "\n";
    }
}

void updateTime() {
    cout << "\nHours remaining before departure: "
         << hoursRemaining << "\n";

    cout << "Enter new hours remaining: ";
    cin >> hoursRemaining;

    if (hoursRemaining < 0) {
        hoursRemaining = 0;
        cout << "Time cannot be negative. Set to 0.\n";
    }

    if (hoursRemaining <= 120)
        cout << "Lower protection period has ended.\n";
    else
        cout << "Protection may apply when occupancy reaches 50%.\n";

    // Existing bookings stay unchanged.
    processWaitingList();
}

int main() {
    for (int c = 0; c < COACHES; c++) {
        for (int s = 0; s < SEATS; s++) {
            seats[c][s].passengerIndex = -1;
        }
    }

    int choice;

    do {
        cout << "\n====================================\n";
        cout << " SMART SEAT BERTH ALLOCATION SYSTEM\n";
        cout << "====================================\n";
        cout << "1. Book Ticket\n";
        cout << "2. View Seat Status\n";
        cout << "3. View All Bookings\n";
        cout << "4. Cancel Ticket\n";
        cout << "5. View Waiting List\n";
        cout << "6. Update Time Before Departure\n";
        cout << "0. Exit\n";
        cout << "Enter choice: ";
        cin >> choice;

        switch (choice) {
            case 1:
                bookTicket();
                break;

            case 2:
                showSeats();
                break;

            case 3:
                showBookings();
                break;

            case 4:
                cancelTicket();
                break;

            case 5:
                showWaitingList();
                break;

            case 6:
                updateTime();
                break;

            case 0:
                cout << "Thank you for using the system!\n";
                break;

            default:
                cout << "Invalid choice. Try again.\n";
        }

    } while (choice != 0);

    return 0;
}
