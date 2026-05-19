//
// Created by matthew on 5/21/25.
//

#ifndef CONSOLEIO_H
#define CONSOLEIO_H
#include <cstdint>
#include <iostream>

#include "streamio.h"


/**
 * ConsoleHandle class for handling console input/output in raw mode
 */
class ConsoleHandle final : public StreamHandle {

    /**
     * The start of the unmodifiable code region
     */
    uint32_t inputBase = 0;

    /**
     * The current position of the input cursor
     */
    uint32_t inputCursor = 0;

    /**
     * Flag indicating whether raw mode is enabled
     */
    bool rawModeEnabled = false;

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

    /**
     * Outputs a character to the console
     * @param c The character to output
     */
    void putChar(char c) override;
};

#endif // CONSOLEIO_H
