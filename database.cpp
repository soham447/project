#include "database.h"

#include <fstream>
#include <sstream>
#include <chrono>
#include <ctime>
#include <iomanip>

using namespace std;

// Prompt banks 

const vector<string> ENGLISH_PROMPTS = {
    "the quick brown fox jumps over the lazy dog near the river bank",
    "programming is the art of telling another human what one wants the computer to do",
    "success is not final failure is not fatal it is the courage to continue that counts",
    "in the middle of every difficulty lies opportunity and hard work pays off",
    "the only way to do great work is to love what you do and never give up"
};

const vector<string> CODING_PROMPTS = {
    "int main() { return 0; }",
    "for (int i = 0; i < n; i++) { sum += arr[i]; }",
    "vector<int> v = {1, 2, 3, 4, 5};",
    "if (x > 0 && y != nullptr) { delete[] y; y = new int[x]; }",
    "auto it = find(v.begin(), v.end(), target);"
};

string pickPrompt(const vector<string>& prompts) {
    static int idx = 0;
    return prompts[idx++ % (int)prompts.size()];
}

// Persistence 

const string DB_FILE = "typing_db.csv";

string nowTimestamp() {
    auto t  = chrono::system_clock::to_time_t(chrono::system_clock::now());
    tm* tm = localtime(&t);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm);
    return string(buf);
}

void saveResult(const string& name, const TestResult& r) {
    ofstream f(DB_FILE, ios::app);
    f << name              << ","
      << r.testType        << ","
      << fixed << setprecision(2)
      << r.wpm             << ","
      << r.accuracy        << ","
      << r.misspelled      << ","
      << r.totalWords      << ","
      << r.timestamp       << ","
      << r.rawMisspelled   << "\n";
}

map<string, Player> loadDatabase() {
    map<string, Player> db;
    ifstream f(DB_FILE);
    if (!f.is_open()) return db;

    string line;
    while (getline(f, line)) {
        if (line.empty()) continue;
        vector<string> parts;
        stringstream ss(line);
        string tok;
        int col = 0;
        while (getline(ss, tok, ',') && col < 7) { parts.push_back(tok); ++col; }
        string rest; getline(ss, rest); parts.push_back(rest);
        if (parts.size() < 8) continue;

        TestResult tr;
        tr.testType      = parts[1];
        tr.wpm           = stod(parts[2]);
        tr.accuracy      = stod(parts[3]);
        tr.misspelled    = stoi(parts[4]);
        tr.totalWords    = stoi(parts[5]);
        tr.timestamp     = parts[6];
        tr.rawMisspelled = parts[7];

        string pname = parts[0];
        db[pname].name    = pname;
        db[pname].history.push_back(tr);
    }
    return db;
}
