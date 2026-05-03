#pragma once

#include "database.h"
#include <map>
#include <string>

// Core typing test

TestResult runTypingTest(const std::string& prompt, const std::string& testType);

// Post-test display

void displayResult(const std::string& name, const TestResult& r);

// Leaderboard

void showLeaderboard(const std::map<std::string, Player>& db,
                     const std::string& filterType = "");

// Per-player views

void showHistory(const Player& player);
void showAnalysis(const Player& player);
