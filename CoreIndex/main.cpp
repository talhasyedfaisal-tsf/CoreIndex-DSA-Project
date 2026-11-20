#include "CoreIndex.h"
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

static void printMenu() {
    cout << "\n================ CoreIndex ================\n"
         << "1. Add record\n"
         << "2. Search by ID\n"
         << "3. Update record\n"
         << "4. Delete record\n"
         << "5. View category order\n"
         << "6. View category range\n"
         << "7. Dispatch highest-priority record\n"
         << "8. Show record count\n"
         << "0. Exit\n"
         << "Choose: ";
}

static bool readInt(const char* prompt, int& value) {
    cout << prompt;
    string input;
    if (!getline(cin, input)) {
        cout << "Invalid integer." << endl;
        return false;
    }

    istringstream parser(input);
    char extra;
    if (!(parser >> value) || (parser >> extra)) {
        cout << "Invalid integer." << endl;
        return false;
    }
    return true;
}

static bool readText(const char* prompt, string& text) {
    cout << prompt;
    if (!getline(cin, text)) return false;

    size_t first = text.find_first_not_of(" \t\r\n");
    if (first == string::npos) {
        text.clear();
        return false;
    }

    size_t last = text.find_last_not_of(" \t\r\n");
    text = text.substr(first, last - first + 1);
    return true;
}

static bool readPriority(int& priority) {
    if (!readInt("Priority (0-100): ", priority)) return false;
    if (priority < 0 || priority > 100) {
        cout << "Priority must be from 0 to 100." << endl;
        return false;
    }
    return true;
}

int main() {
    CoreIndex index;
    int choice;

    index.addRecord(1001, "Server outage", "Infrastructure", 95);
    index.addRecord(1002, "Card reader failure", "Hardware", 82);
    index.addRecord(1003, "Database slow", "Software", 88);
    index.addRecord(1004, "Network gateway loss", "Infrastructure", 97);
    index.addRecord(1005, "Security alert", "Security", 100);

    for (;;) {
        printMenu();
        if (!readInt("", choice)) continue;

        if (choice == 0) break;

        if (choice == 1) {
            int id;
            int priority;
            string name;
            string category;

            if (!readInt("ID: ", id)) continue;
            if (!readText("Name: ", name)) {
                cout << "Name cannot be empty." << endl;
                continue;
            }
            if (!readText("Category: ", category)) {
                cout << "Category cannot be empty." << endl;
                continue;
            }
            if (!readPriority(priority)) continue;
            if (name.size() >= 64) {
                cout << "Name must be at most 63 characters." << endl;
                continue;
            }
            if (category.size() >= 32) {
                cout << "Category must be at most 31 characters." << endl;
                continue;
            }

            if (id < 0)
                cout << "ID cannot be negative." << endl;
            else if (index.addRecord(id, name.c_str(), category.c_str(), priority))
                cout << "Record added." << endl;
            else
                cout << "Record ID already exists." << endl;

        } else if (choice == 2) {
            int id;
            if (readInt("ID: ", id)) {
                Record* record = index.findRecord(id);
                if (record)
                    cout << '[' << record->id << "] " << record->name << " | "
                         << record->category << " | priority " << record->priority << endl;
                else
                    cout << "Record not found." << endl;
            }

        } else if (choice == 3) {
            int id;
            int priority;
            string name;
            string category;

            if (!readInt("ID: ", id)) continue;
            if (!index.findRecord(id)) {
                cout << "Record not found." << endl;
                continue;
            }
            if (!readText("New name: ", name)) {
                cout << "Name cannot be empty." << endl;
                continue;
            }
            if (!readText("New category: ", category)) {
                cout << "Category cannot be empty." << endl;
                continue;
            }
            if (!readPriority(priority)) continue;
            if (name.size() >= 64) {
                cout << "Name must be at most 63 characters." << endl;
                continue;
            }
            if (category.size() >= 32) {
                cout << "Category must be at most 31 characters." << endl;
                continue;
            }

            if (index.updateRecord(id, name.c_str(), category.c_str(), priority))
                cout << "Record updated." << endl;
            else
                cout << "Update failed." << endl;

        } else if (choice == 4) {
            int id;
            if (readInt("ID: ", id)) {
                if (index.deleteRecord(id))
                    cout << "Record deleted." << endl;
                else
                    cout << "Record not found." << endl;
            }

        } else if (choice == 5) {
            index.printAllByCategory();

        } else if (choice == 6) {
            string low;
            string high;
            if (!readText("Low category: ", low) || !readText("High category: ", high)) {
                cout << "Category range cannot contain empty values." << endl;
            } else {
                index.printCategoryRange(low.c_str(), high.c_str());
            }

        } else if (choice == 7) {
            Record* record = nullptr;
            if (index.dispatchNext(record)) {
                cout << "Dispatched: [" << record->id << "] " << record->name << " | "
                     << record->category << " | priority " << record->priority << endl;
                delete record;
            } else {
                cout << "No records available." << endl;
            }

        } else if (choice == 8) {
            cout << "Current record count: " << index.recordCount() << endl;

        } else {
            cout << "Unknown option." << endl;
        }
    }

    cout << "CoreIndex closed." << endl;
    return 0;
}
