//
// Created by matthew on 5/21/25.
//

#ifndef CONSOLEIO_H
#define CONSOLEIO_H
#include <cstdint>


/**
 * ConsoleHandle class for handling console input/output in raw mode
 */
class ConsoleHandle {

    /**
     * The start of the unmodifiable code region
     */
    uint32_t inputBase = 0;

    /**
     * The current position of the input cursor
     */
    uint32_t inputCursor = 0;

public:
    /**
     * Enable raw mode for terminal input to get single characters without newline
     */
    void enableRawConsoleMode();


    /**
     * Disable raw mode for terminal input to restore default behavior
     */
    void disableRawConsoleMode();


    /**
     * Check if there are characters available to read from the console
     * @return True if there are characters available, false otherwise
     */
    bool consoleHasChar();


    /**
     * Get a single character from the console
     * @return The character read from the console
     * @throw runtime_error if there is no character to read from the console
     */
    char consoleGetChar();
};

#endif // CONSOLEIO_H
