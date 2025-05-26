//
// Created by matthew on 5/21/25.
//

#include "io/consoleio.h"

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
