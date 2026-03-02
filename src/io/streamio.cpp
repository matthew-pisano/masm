//
// Created by matthew on 6/8/25.
//

#include "io/streamio.h"

#include <unistd.h>


void StreamHandle::edit(const bool set) { isEditing = set; }

bool StreamHandle::editing() const { return isEditing; }

std::string StreamHandle::getBuffer() { return buffer; }

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

std::string StreamHandle::getLine() {
    while (true)
        if (getCharBlocking() == '\n')
            break;
    return buffer.substr(0, buffer.size() - 1);
}

void StreamHandle::putChar(const char c) {
    buffer.insert(cursor + 1, 1, c);
    cursor++;
    if (!isEditing)
        flush();
}

void StreamHandle::putStr(const std::string& str) {
    for (const char c : str)
        putChar(c);
}

void StreamHandle::clear() {
    buffer.clear();
    cursor = -1;
}

void StreamHandle::flush() {
    show();
    clear();
}

void StreamHandle::seek(const int32_t offset, const Whence whence) {
    // If cursor is uninitialized, do nothing
    if (cursor == -1)
        return;

    const int32_t buffSize = static_cast<int32_t>(buffer.size());
    switch (whence) {
        case Whence::SET:
            cursor = offset;
            break;
        case Whence::CUR:
            cursor += offset;
            break;
        case Whence::END:
            cursor = buffSize + offset;
            break;
    }

    // Clamp cursor within valid bounds
    if (cursor > buffSize)
        cursor = buffSize;
    else if (cursor < 0)
        cursor = 0;
}


void StreamHandle::show() { ostream << buffer << std::flush; }
