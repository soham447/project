#pragma once

#include <string>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

// Terminal size & display helpers

int         termWidth();
void        printLine(char c = '-');
void        printCentered(const std::string& text);
void        clearScreen();
void        waitKey();

// Raw-mode RAII wrapper

struct RawMode {
    termios saved{};
    RawMode();
    ~RawMode();
};

// Input helpers

std::string readLineRaw();   // echoed, backspace-aware line input
char        readChar();      // single keypress, no echo
int         readMenuChoice(); // digit + Enter, with echo
