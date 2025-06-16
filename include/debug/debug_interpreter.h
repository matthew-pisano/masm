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
     * The ID of the next breakpoint to be set.  This is used to ensure that each breakpoint has a
     * unique ID
     */
    size_t nextBreakpoint = 1;

    /**
     * A map of breakpoints, where the key is the instruction address and the value is the
     * breakpoint ID
     */
    std::map<uint32_t, size_t> breakpoints;

    /**
     * Parses a debug command from th user
     * @param cmdStr The command to parse
     * @param layout The memory layout to use for the command
     * @return Whether to prompt the user for another command
     */
    bool parseCommand(const std::string& cmdStr, const MemLayout& layout);

    /**
     * Validates the arguments for a given command
     * @param cmd The command to validate arguments for
     * @param args The arguments to validate
     * @return True if the arguments are valid, false otherwise
     */
    bool validateArguments(const std::string& cmd, const std::vector<std::string>& args);

    /**
     * Sets a breakpoint at the given address or source location
     * @param arg
     */
    void setBreakpoint(const std::string& arg);

    /**
     * Deletes a breakpoint by its ID
     * @param arg The ID of the breakpoint to delete, or an empty string to delete all breakpoints
     */
    void deleteBreakpoint(const std::string& arg);

    /**
     * Lists all breakpoints currently set in the interpreter
     */
    void listBreakpoints();

    /**
     * Lists all labels in the program, including their addresses and names
     */
    void listLabels();

    /**
     * Lists all registers in the interpreter, including general-purpose and special-purpose
     */
    void listRegisters();

    /**
     * Lists all Co-Processor 0 registers
     */
    void listCP0Registers();

    /**
     * Lists all Co-Processor 1 registers
     */
    void listCP1Registers();

    /**
     * Examines the memory at the specified address and prints its value
     * @param arg The address to examine, in hex format (0x...)
     */
    void examineAddress(const std::string& arg);

    /**
     * Prints the value of a register
     * @param arg The name of the register to print
     */
    void printRegister(const std::string& arg);

    /**
     * Prints the value of a label in the program
     * @param arg The name of the label to print
     */
    void printLabel(const std::string& arg);

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
