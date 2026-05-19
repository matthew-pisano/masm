//
// Created by matthew on 6/8/25.
//

#include "io/streamio.h"

#include <unistd.h>

bool StreamHandle::hasChar() {
    const bool hasChar = istream.peek() != std::istream::traits_type::eof();
    // Clear error flags from peeking when stream is empty
    istream.clear();
    return hasChar;
}

char StreamHandle::getChar() {
    char c;
    istream.get(c);
    if (istream.eof())
        throw std::runtime_error("End of input stream reached");
    return c;
}

char StreamHandle::getCharBlocking() {
    while (!hasChar())
        usleep(1000); // Sleep for 1 ms
    return getChar();
}

void StreamHandle::putChar(const char c) {
    ostream.put(c);
    if (ostream.fail())
        throw std::runtime_error("Failed to write character to output stream");
}

void StreamHandle::putStr(const std::string& str) {
    for (const char c : str)
        putChar(c);
}
