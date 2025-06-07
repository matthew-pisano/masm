//
// Created by matthew on 5/21/25.
//

#include "io/consoleio.h"

#ifdef _WIN32
#include <conio.h>
#include <stdexcept>
#include <windows.h>


// Store original console mode to restore later
static DWORD originalConsoleMode = 0;
static HANDLE hStdin = nullptr;
static bool rawModeEnabled = false;


void enableRawConsoleMode() {
    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if (hStdin == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("Failed to get stdin handle");
    }

    // Get the current console mode
    if (!GetConsoleMode(hStdin, &originalConsoleMode)) {
        throw std::runtime_error("Failed to get console mode");
    }

    // Disable line input and echo input
    DWORD newMode = originalConsoleMode;
    newMode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);

    // Set the new console mode
    if (!SetConsoleMode(hStdin, newMode)) {
        throw std::runtime_error("Failed to set console mode");
    }

    rawModeEnabled = true;
}


void disableRawConsoleMode() {
    if (rawModeEnabled && hStdin != nullptr) {
        // Restore the original console mode
        SetConsoleMode(hStdin, originalConsoleMode);
        rawModeEnabled = false;
    }
}


bool consoleHasChar() {
    if (!rawModeEnabled) {
        return false;
    }

    // Check if there are input events available
    DWORD numEvents = 0;
    if (!GetNumberOfConsoleInputEvents(hStdin, &numEvents)) {
        return false;
    }

    if (numEvents == 0) {
        return false;
    }

    // Peek at the input events to see if any are key events
    INPUT_RECORD inputRecord;
    DWORD eventsRead = 0;

    if (!PeekConsoleInput(hStdin, &inputRecord, 1, &eventsRead)) {
        return false;
    }

    if (eventsRead == 0) {
        return false;
    }

    // Check if it's a key down event (not key up)
    if (inputRecord.EventType == KEY_EVENT && inputRecord.Event.KeyEvent.bKeyDown &&
        inputRecord.Event.KeyEvent.uChar.AsciiChar != 0) {
        return true;
    }

    // If it's not a useful key event, consume it and check again
    ReadConsoleInput(hStdin, &inputRecord, 1, &eventsRead);

    return false;
}


char consoleGetChar() {
    if (!rawModeEnabled) {
        throw std::runtime_error("Raw console mode not enabled");
    }

    while (true) {
        INPUT_RECORD inputRecord;
        DWORD eventsRead = 0;

        // Read input event
        if (!ReadConsoleInput(hStdin, &inputRecord, 1, &eventsRead)) {
            throw std::runtime_error("Failed to read console input");
        }

        if (eventsRead == 0) {
            throw std::runtime_error("No input events available");
        }

        // Check if it's a key down event with a valid character
        if (inputRecord.EventType == KEY_EVENT && inputRecord.Event.KeyEvent.bKeyDown &&
            inputRecord.Event.KeyEvent.uChar.AsciiChar != 0) {
            return inputRecord.Event.KeyEvent.uChar.AsciiChar;
        }

        // If not a useful key event, continue reading
    }
}

#else
// Linux implementation
#include <fcntl.h>
#include <stdexcept>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>


void enableRawConsoleMode() {
    termios term{};
    tcgetattr(STDIN_FILENO, &term);
    // Turn off canonical mode and echo mode
    term.c_lflag &= ~(ICANON);
    // Set minimum number of input bytes and timeout
    term.c_cc[VMIN] = 0; // Return immediately, even if no bytes are available
    term.c_cc[VTIME] = 0; // No timeout
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &term);

    // Set stdin to non-blocking mode
    const int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
}


void disableRawConsoleMode() {
    termios term{};
    tcgetattr(STDIN_FILENO, &term);
    // Restore canonical mode and echo mode
    term.c_lflag |= (ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &term);

    // Reset stdin to blocking mode
    const int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK);
}


bool consoleHasChar() {
    // Try to read 0 bytes - will return > 0 if data is available
    char buf;
    const size_t bytesRead = read(STDIN_FILENO, &buf, 0);
    if (bytesRead > 0)
        // Rare case: we actually got data despite reading 0 bytes
        // This shouldn't normally happen
        return true;

    // Check if there are bytes to read without actually reading them
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    timeval tv{};
    tv.tv_sec = 0;
    tv.tv_usec = 0; // Return immediately

    // Check if there's data available to read
    return select(STDIN_FILENO + 1, &readfds, nullptr, nullptr, &tv) > 0;
}


char consoleGetChar() {
    char c;
    const size_t bytesRead = read(STDIN_FILENO, &c, 1);

    if (bytesRead > 0)
        return c;

    throw std::runtime_error("No character available to read from console");
}

#endif
