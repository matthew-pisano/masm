//
// Created by matthew on 6/15/25.
//

#ifndef DEBUG_INTERPRETER_H
#define DEBUG_INTERPRETER_H
#include "interpreter/interpreter.h"

/**
 * Utility class that allows tests to access the internal state of an interpreter
 */
class DebugInterpreter : public Interpreter {

public:
    /**
     * Constructor for the DebugInterpreter class
     * @param ioMode The I/O mode to use for the interpreter
     * @param streamHandle The stream handle to use for I/O operations
     */
    DebugInterpreter(const IOMode ioMode, StreamHandle& streamHandle) :
        Interpreter(ioMode, streamHandle) {}

    /**
     * Constructor for the DebugInterpreter class
     * @param ioMode The I/O mode to use for the interpreter
     * @param streamHandle The stream handle to use for I/O operations
     * @param useLittleEndian Whether to use little-endian byte order for memory layout
     */
    DebugInterpreter(const IOMode ioMode, StreamHandle& streamHandle, const bool useLittleEndian) :
        Interpreter(ioMode, streamHandle, useLittleEndian) {}

    /**
     * Gets the current state of the interpreter
     * @return The current state of the interpreter
     */
    State& getState();
};

#endif //DEBUG_INTERPRETER_H
