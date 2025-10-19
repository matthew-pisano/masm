//
// Created by matthew on 5/21/25.
//

#ifndef CONSOLEIO_H
#define CONSOLEIO_H
#include <iostream>

#include "streamio.h"


enum class KeyboardEscape {
    UP = 'A',
    DOWN = 'B',
    RIGHT = 'C',
    LEFT = 'D',
    HOME = 'H',
    END = 'F',
    DELETE = '3',
    PGUP = '5',
    PGDN = '6'
};


/**
 * ConsoleHandle class for handling console input/output in raw mode
 */
class ConsoleHandle final : public StreamHandle {

    /**
     * Size of the last written buffer
     */
    size_t writtenBufferSize = 0;

    /**
     * Last character read from the console
     */
    char lastReadChar = 0;

    /**
     * Flag indicating whether raw mode is enabled
     */
    bool rawModeEnabled = false;

    /**
     * Deletes a character from the buffer at the current cursor
     */
    void delChar();

public:
    ConsoleHandle() : StreamHandle(std::cin, std::cout) {}

    /**
     * Enable raw mode for terminal input to get single characters without newline
     */
    void enableRawConsoleMode();
    /**
     * Disable raw mode for terminal input to restore default behavior
     */
    void disableRawConsoleMode();

    /**
     * Checks if there are characters available to read from the console
     * @return True if there are characters available, false otherwise
     */
    [[nodiscard]] bool hasChar() override;

    /**
     * Gets a character from the console input
     * @return The character read from the console
     */
    [[nodiscard]] char getChar() override;

    void putChar(char c) override;

    /**
     * Clears the current input line from the console
     */
    void clear() override;

    void show() override;
};

#endif // CONSOLEIO_H
