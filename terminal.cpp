#include "terminal.h"

#include <iostream>

using namespace std;

// Terminal size & display helpers 

int termWidth() {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return (w.ws_col > 0) ? w.ws_col : 80;
}

void printLine(char c) {
    int w = termWidth();
    for (int i = 0; i < w; ++i) cout << c;
    cout << "\n";
}

void printCentered(const string& text) {
    int w   = termWidth();
    int pad = (w - (int)text.size()) / 2;
    if (pad < 0) pad = 0;
    cout << string(pad, ' ') << text << "\n";
}

void clearScreen() {
    cout << "\033[2J\033[H";
    cout.flush();
}

void waitKey() {
    cout << "\n  Press any key to continue...";
    cout.flush();
    char c = 0;
    RawMode rm;
    if (read(STDIN_FILENO, &c, 1) < 0) c = 0;
    cout << "\n";
}

// Raw-mode RAII wrapper 

RawMode::RawMode() {
    tcgetattr(STDIN_FILENO, &saved);
    termios raw  = saved;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN]  = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

RawMode::~RawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &saved);
}

// Input helpers

string readLineRaw() {
    string buf;
    RawMode rm;
    while (true) {
        char c;
        if (read(STDIN_FILENO, &c, 1) != 1) break;
        if (c == '\r' || c == '\n') { cout << "\n"; cout.flush(); break; }
        if (c == 127 || c == '\b') {
            if (!buf.empty()) { buf.pop_back(); cout << "\b \b"; cout.flush(); }
        } else if (c >= 32) {
            buf += c; cout << c; cout.flush();
        }
    }
    return buf;
}

char readChar() {
    char c = 0;
    RawMode rm;
    if (read(STDIN_FILENO, &c, 1) < 0) return 0;
    return c;
}

int readMenuChoice() {
    string buf;
    RawMode rm;
    while (true) {
        char c;
        if (read(STDIN_FILENO, &c, 1) != 1) continue;
        if (c == '\r' || c == '\n') {
            cout << "\n"; cout.flush();
            if (buf.size() == 1 && buf[0] >= '0' && buf[0] <= '9')
                return buf[0] - '0';
            buf.clear();
            cout << "  Choice: "; cout.flush();
        } else if (c == 127 || c == '\b') {
            if (!buf.empty()) { buf.pop_back(); cout << "\b \b"; cout.flush(); }
        } else if (c >= '0' && c <= '9' && buf.empty()) {
            buf += c; cout << c; cout.flush();
        }
    }
}
