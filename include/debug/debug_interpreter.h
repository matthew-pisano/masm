//
// Created by matthew on 6/15/25.
//

#ifndef DEBUG_INTERPRETER_H
#define DEBUG_INTERPRETER_H
#include "interpreter/interpreter.h"

/**
 * Utility class that allows tests to access the internal state of an interpreter
 */
class DebugInterpreter final : public Interpreter {
    /**
     * Whether the debug interpreter is in interactive mode, allowing user input to control the flow
     * of the program
     */
    bool isInteractive = false;

    /**
     * A map of breakpoints, where the key is the instruction address and the value is the
     * breakpoint ID
     */
    std::map<uint32_t, size_t> breakpoints;

    /**
     * Parses a debug command from th user
     * @param cmd The command to parse
     * @return Whether to prompt the user for another command
     */
    bool parseCommand(std::string cmd);

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

    /**
     * Sets whether the interpreter is in interactive mode
     * @param interactive True to set the interpreter to interactive mode, false otherwise
     */
    void setInteractive(bool interactive);

    /**
     * Initializes a program and steps until an exit syscall or exception occurs.  Additionally,
     * user input is taken to control the flow of the program
     * @param layout The initial memory layout to use for loading in the program and data
     * @return The exit code of the program
     */
    int interpret(const MemLayout& layout) override;
};

#endif // DEBUG_INTERPRETER_H
