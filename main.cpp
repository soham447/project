//  main.cpp  —  entry point: menus only
//
//  Modules used:
//    terminal.h / terminal.cpp  — terminal helpers, raw input
//    database.h / database.cpp  — data structures, CSV persistence, prompts
//    typing.h   / typing.cpp    — typing test, results, leaderboard, history, analysis
//
//  Build:
//    g++ -O2 main.cpp terminal.cpp database.cpp typing.cpp -o typing_analyzer

#include "terminal.h"
#include "database.h"
#include "typing.h"

#include <iostream>
#include <algorithm>    // replace

using namespace std;

// Player sub-menu

static void playerMenu(const string& name) {
    while (true) {
        clearScreen();
        printLine('-');
        printCentered("Logged in as: " + name);
        printLine('-');
        cout << "\n"
                  << "  [1] English Typing Test\n"
                  << "  [2] Coding Style Test\n"
                  << "  [3] My History\n"
                  << "  [4] My Analysis\n"
                  << "  [5] Global Leaderboard\n"
                  << "  [6] Leaderboard - English only\n"
                  << "  [7] Leaderboard - Coding only\n"
                  << "  [0] Logout\n"
                  << "\n  Choice: ";
        cout.flush();

        int choice = readMenuChoice();

        if (choice == 1 || choice == 2) {
            string type  = (choice == 1) ? "english" : "coding";
            auto& prompts     = (choice == 1) ? ENGLISH_PROMPTS : CODING_PROMPTS;
            TestResult res    = runTypingTest(pickPrompt(prompts), type);
            saveResult(name, res);
            displayResult(name, res);
            waitKey();

        } else if (choice == 3) {
            auto db = loadDatabase();
            auto it = db.find(name);
            if (it != db.end()) showHistory(it->second);
            else { clearScreen(); printCentered("No history found.\n"); }
            waitKey();

        } else if (choice == 4) {
            auto db = loadDatabase();
            auto it = db.find(name);
            if (it != db.end()) showAnalysis(it->second);
            else { clearScreen(); printCentered("No history found.\n"); }
            waitKey();

        } else if (choice == 5) { auto db = loadDatabase(); showLeaderboard(db);             waitKey(); }
          else if (choice == 6) { auto db = loadDatabase(); showLeaderboard(db, "english");  waitKey(); }
          else if (choice == 7) { auto db = loadDatabase(); showLeaderboard(db, "coding");   waitKey(); }
          else if (choice == 0) { break; }
    }
}

// Main menu

int main() {
    while (true) {
        clearScreen();
        printLine('=');
        printCentered("TYPING ANALYZER");
        printCentered("WPM  /  Accuracy  /  Leaderboard  /  History");
        printLine('=');
        cout << "\n"
                  << "  [1] Login / Register\n"
                  << "  [2] View Global Leaderboard\n"
                  << "  [0] Exit\n"
                  << "\n  Choice: ";
        cout.flush();

        int choice = readMenuChoice();

        if (choice == 1) {
            clearScreen();
            cout << "\n  Enter your name: ";
            cout.flush();
            string name = readLineRaw();
            size_t s = name.find_first_not_of(" \t");
            size_t e = name.find_last_not_of(" \t");
            if (s == string::npos || name.empty()) continue;
            name = name.substr(s, e - s + 1);
            replace(name.begin(), name.end(), ' ', '_');
            if (!name.empty()) playerMenu(name);

        } else if (choice == 2) {
            auto db = loadDatabase();
            showLeaderboard(db);
            waitKey();

        } else if (choice == 0) {
            clearScreen();
            printCentered("Goodbye! Keep practicing.");
            cout << "\n";
            break;
        }
    }
    return 0;
}
