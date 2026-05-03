#include "typing.h"
#include "terminal.h"

#include <iostream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <cmath>
#include <unistd.h>

using namespace std;

// Core typing test

TestResult runTypingTest(const string& prompt, const string& testType) {
    clearScreen();
    printLine('=');
    printCentered((testType == "coding") ? "  CODING STYLE TEST" : "  ENGLISH TYPING TEST");
    printLine('=');

    cout << "\n";
    cout << "  Type the text below exactly. Press ENTER when done.\n";
    cout << "  Backspace is supported.\n\n";
    cout << "  >>> " << prompt << " <<<\n\n";
    cout << "  Your input: ";
    cout.flush();

    string typed;
    int  cursor  = 0;
    bool started = false;
    chrono::steady_clock::time_point startTime;

    auto moveCursor = [](int n, bool left) {
        if (n <= 0) return;
        cout << "\033[" << n << (left ? "D" : "C");
        cout.flush();
    };

    auto redrawTail = [&]() {
        for (int i = cursor; i < (int)typed.size(); ++i)
            cout << typed[i];
        cout << " ";
        int back = (int)typed.size() - cursor + 1;
        cout << "\033[" << back << "D";
        cout.flush();
    };

    {
        RawMode rm;
        while (true) {
            char c;
            if (read(STDIN_FILENO, &c, 1) != 1) break;

            // Enter
            if (c == '\r' || c == '\n') {
                moveCursor((int)typed.size() - cursor, false);
                cout << "\n"; cout.flush(); break;
            }

            // Arrow keys  (ESC [ A/B/C/D)
            if (c == '\033') {
                char seq[2] = {0, 0};
                if (read(STDIN_FILENO, &seq[0], 1) < 0) continue;
                if (read(STDIN_FILENO, &seq[1], 1) < 0) continue;
                if (seq[0] == '[') {
                    if (seq[1] == 'C' && cursor < (int)typed.size()) {
                        moveCursor(1, false); ++cursor;
                    } else if (seq[1] == 'D' && cursor > 0) {
                        moveCursor(1, true);  --cursor;
                    }
                }
                continue;
            }

            // Start timer on first real character
            if (!started && c != 127 && c != '\b' && c >= 32) {
                started   = true;
                startTime = chrono::steady_clock::now();
            }

            // Backspace
            if (c == 127 || c == '\b') {
                if (cursor > 0) {
                    typed.erase(cursor - 1, 1);
                    --cursor;
                    cout << "\b";
                    redrawTail();
                }
            }
            // Printable — insert at cursor
            else if (c >= 32) {
                typed.insert(cursor, 1, c);
                ++cursor;
                for (int i = cursor - 1; i < (int)typed.size(); ++i)
                    cout << typed[i];
                int back = (int)typed.size() - cursor;
                if (back > 0) cout << "\033[" << back << "D";
                cout.flush();
            }
        }
    } // RawMode restored here

    auto endTime = chrono::steady_clock::now();
    double elapsed = chrono::duration<double>(endTime - startTime).count();
    if (!started || elapsed < 0.1) elapsed = 0.1;

    auto splitWords = [](const string& s) {
        vector<string> words;
        istringstream iss(s); string w;
        while (iss >> w) words.push_back(w);
        return words;
    };

    auto refWords   = splitWords(prompt);
    auto typedWords = splitWords(typed);

    int total = (int)refWords.size(), misspelled = 0;
    vector<string> wrong;
    for (int i = 0; i < (int)typedWords.size() && i < total; ++i)
        if (typedWords[i] != refWords[i]) { ++misspelled; wrong.push_back(typedWords[i]); }
    if ((int)typedWords.size() < total) misspelled += total - (int)typedWords.size();

    double accuracy = 100.0 * (total - misspelled) / (double)total;
    if (accuracy < 0) accuracy = 0;
    double wpm = ((double)typedWords.size() / elapsed) * 60.0;

    string misspelledStr;
    for (size_t i = 0; i < wrong.size(); ++i) { if (i) misspelledStr += "|"; misspelledStr += wrong[i]; }

    TestResult res;
    res.testType      = testType;
    res.wpm           = wpm;
    res.accuracy      = accuracy;
    res.misspelled    = misspelled;
    res.totalWords    = total;
    res.timestamp     = nowTimestamp();
    res.rawMisspelled = misspelledStr;
    return res;
}

// Post-test display

void displayResult(const string& name, const TestResult& r) {
    clearScreen();
    printLine('=');
    printCentered("RESULT SUMMARY");
    printLine('=');

    cout << "\n"
              << "  Player   : " << name        << "\n"
              << "  Test     : " << r.testType  << "\n"
              << "  Date     : " << r.timestamp << "\n\n";
    printLine('-');

    cout << fixed << setprecision(1)
              << "  WPM      : " << r.wpm       << "\n"
              << "  Accuracy : " << r.accuracy  << "%\n"
              << "  Errors   : " << r.misspelled << " / " << r.totalWords << " words\n";

    if (!r.rawMisspelled.empty()) {
        cout << "\n";
        printLine('-');
        cout << "  Misspelled words:\n  ";
        string s = r.rawMisspelled;
        replace(s.begin(), s.end(), '|', ' ');
        cout << s << "\n";
    }

    cout << "\n";
    printLine('-');
    string badge;
    if      (r.wpm >= 80 && r.accuracy >= 95) badge = "[ ELITE TYPIST ]";
    else if (r.wpm >= 60 && r.accuracy >= 90) badge = "[ ADVANCED ]";
    else if (r.wpm >= 40 && r.accuracy >= 80) badge = "[ INTERMEDIATE ]";
    else if (r.wpm >= 20)                      badge = "[ BEGINNER ]";
    else                                        badge = "[ KEEP PRACTICING ]";
    printCentered(badge);
    cout << "\n";
}

// Leaderboard

void showLeaderboard(const map<string, Player>& db,
                     const string& filterType) {
    clearScreen();
    printLine('=');
    printCentered(filterType.empty() ? "GLOBAL LEADERBOARD - TOP 5"
                                     : "LEADERBOARD: " + filterType + " - TOP 5");
    printLine('=');
    cout << "\n";

    struct Entry { string name; double wpm; double acc; string type; };
    vector<Entry> entries;
    for (auto& [pname, player] : db) {
        double bestWpm = -1, bestAcc = 0; string bestType;
        for (auto& r : player.history) {
            if (!filterType.empty() && r.testType != filterType) continue;
            if (r.wpm > bestWpm) { bestWpm = r.wpm; bestAcc = r.accuracy; bestType = r.testType; }
        }
        if (bestWpm >= 0) entries.push_back({pname, bestWpm, bestAcc, bestType});
    }
    sort(entries.begin(), entries.end(),
              [](const Entry& a, const Entry& b){ return a.wpm > b.wpm; });

    int shown = min((int)entries.size(), 5);
    if (shown == 0) { printCentered("No data available yet."); cout << "\n"; return; }

    cout << "  " << left
              << setw(4)  << "#"
              << setw(20) << "Name"
              << setw(10) << "Best WPM"
              << setw(12) << "Accuracy"
              << setw(12) << "Test Type" << "\n";
    printLine('-');
    for (int i = 0; i < shown; ++i) {
        ostringstream accStr;
        accStr << fixed << setprecision(1) << entries[i].acc << "%";
        cout << "  " << left
                  << setw(4)  << (i + 1)
                  << setw(20) << entries[i].name
                  << fixed << setprecision(1)
                  << setw(10) << entries[i].wpm
                  << setw(12) << accStr.str()
                  << setw(12) << entries[i].type << "\n";
    }
    cout << "\n";
}

// History

void showHistory(const Player& player) {
    clearScreen();
    printLine('=');
    printCentered("HISTORY FOR: " + player.name);
    printLine('=');
    cout << "\n";

    if (player.history.empty()) { printCentered("No tests recorded yet."); cout << "\n"; return; }

    cout << "  " << left
              << setw(22) << "Timestamp"
              << setw(10) << "Type"
              << setw(10) << "WPM"
              << setw(12) << "Accuracy"
              << setw(8)  << "Errors" << "\n";
    printLine('-');
    for (auto& r : player.history) {
        cout << "  " << left
                  << setw(22) << r.timestamp
                  << setw(10) << r.testType
                  << fixed << setprecision(1)
                  << setw(10) << r.wpm
                  << setw(10) << r.accuracy << "%"
                  << setw(8)  << r.misspelled << "\n";
    }
    printLine('-');
    double avgWpm = 0, avgAcc = 0;
    for (auto& r : player.history) { avgWpm += r.wpm; avgAcc += r.accuracy; }
    avgWpm /= player.history.size();
    avgAcc /= player.history.size();
    cout << "\n  Averages -> WPM: " << fixed << setprecision(1)
              << avgWpm << "   Accuracy: " << avgAcc << "%\n\n";
}

// Analysis

void showAnalysis(const Player& player) {
    clearScreen();
    printLine('=');
    printCentered("TYPING ANALYSIS: " + player.name);
    printLine('=');
    cout << "\n";

    auto analyze = [&](const string& type) {
        vector<TestResult> subset;
        for (auto& r : player.history)
            if (r.testType == type) subset.push_back(r);
        if (subset.empty()) return;

        cout << "  -- " << (type == "coding" ? "CODING TEST" : "ENGLISH TEST") << " --\n";
        printLine('-');

        double sumWpm = 0, sumAcc = 0, maxWpm = 0, minWpm = 1e9;
        int totalErrors = 0, totalWords = 0;
        map<string, int> errorFreq;
        for (auto& r : subset) {
            sumWpm += r.wpm; sumAcc += r.accuracy;
            if (r.wpm > maxWpm) maxWpm = r.wpm;
            if (r.wpm < minWpm) minWpm = r.wpm;
            totalErrors += r.misspelled; totalWords += r.totalWords;
            istringstream ss(r.rawMisspelled); string w;
            while (getline(ss, w, '|')) if (!w.empty()) errorFreq[w]++;
        }
        double avgWpm  = sumWpm / subset.size();
        double avgAcc  = sumAcc / subset.size();
        double errRate = totalWords ? 100.0 * totalErrors / totalWords : 0;

        cout << fixed << setprecision(1)
                  << "  Tests taken    : " << subset.size()   << "\n"
                  << "  Avg WPM        : " << avgWpm          << "\n"
                  << "  Best WPM       : " << maxWpm          << "\n"
                  << "  Worst WPM      : " << minWpm          << "\n"
                  << "  Avg Accuracy   : " << avgAcc << "%"   << "\n"
                  << "  Error Rate     : " << errRate << "%"  << "\n";

        if ((int)subset.size() >= 6) {
            double early = 0, late = 0;
            for (int i = 0; i < 3; ++i) early += subset[i].wpm;
            for (int i = (int)subset.size() - 3; i < (int)subset.size(); ++i) late += subset[i].wpm;
            early /= 3; late /= 3;
            cout << "  Trend          : " << (late > early ? "IMPROVING" : "DECLINING") << "\n";
        }
        if (!errorFreq.empty()) {
            vector<pair<int, string>> sorted;
            for (auto& [w, cnt] : errorFreq) sorted.push_back({cnt, w});
            sort(sorted.rbegin(), sorted.rend());
            cout << "  Most missed    : ";
            int shown = 0;
            for (auto& [cnt, w] : sorted) { if (shown++ >= 5) break; cout << w << "(" << cnt << ") "; }
            cout << "\n";
        }
        cout << "\n";
    };

    analyze("english");
    analyze("coding");
}
