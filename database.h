#pragma once

#include <string>
#include <vector>
#include <map>

using namespace std;

// Data structures 

struct TestResult {
    string testType;
    double      wpm;
    double      accuracy;
    int         misspelled;
    int         totalWords;
    string timestamp;
    string rawMisspelled;
};

struct Player {
    string             name;
    vector<TestResult> history;
};

// Prompt banks 

extern const vector<string> ENGLISH_PROMPTS;
extern const vector<string> CODING_PROMPTS;

string pickPrompt(const vector<string>& prompts);

// Persistence (flat CSV) 

extern const string DB_FILE;

string nowTimestamp();
void        saveResult(const string& name, const TestResult& r);
map<string, Player> loadDatabase();
