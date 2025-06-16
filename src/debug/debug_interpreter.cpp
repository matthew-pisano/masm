//
// Created by matthew on 6/15/25.
//

#include "debug/debug_interpreter.h"

#include <sstream>

#include "exceptions.h"


const std::string debuggerHelp =
        "Debug Interpreter Commands:\n\n"
        "help, h - Show this help message\n"
        "step, s - Execute the next instruction\n"
        "next, n - Execute the next instruction, skipping over procedure calls\n"
        "continue, cont, c - Continue execution until the next breakpoint\n"
        "break, b <address> - Set a breakpoint at the specified address\n"
        "break, b <line> - Set a breakpoint at the specified line number within the current file\n"
        "break, b <file::line> - Set a breakpoint at the specified file and line number\n"
        "delete, d - Delete all breakpoints\n"
        "delete, d <num> - Delete the breakpoint with the specified number\n"
        "list, ls, l - List the lines surrounding the current instruction\n"
        "frame, f - Show the current stack frame\n"
        "finish - Execute until the end of the current procedure (the location stored in $ra)\n"
        "info, i breakpoints - List all breakpoints\n"
        "info, i registers - List all registers and their values\n"
        "info, i cp0 - List all Co-Processor 0 registers and their values\n"
        "info, i cp1 - List all Co-Processor 1 registers and their values\n"
        "x <address> - Examine memory at the specified address\n"
        "print, p <$register> - Print the value of the specified register\n"
        "print, p <label> - Print the value of the specified label\n"
        "exit - Exit the debugger\n";


State& DebugInterpreter::getState() { return state; }

void DebugInterpreter::setInteractive(const bool interactive) { isInteractive = interactive; }

int DebugInterpreter::interpret(const MemLayout& layout) {
    initProgram(layout);

    // Set initial breakpoint at start of program
    breakpoints[0] = state.registers[Register::PC];

    while (true) {
        try {
            // Skip interactive debug command input if not in interactive mode
            if (isInteractive) {
                uint32_t pc = state.registers[Register::PC];
                bool getCommand =
                        std::find(breakpoints.begin(), breakpoints.end(), pc) != breakpoints.end();
                // Clear system breakpoint
                breakpoints.erase(0);
                // Get user commands until none are expected
                while (getCommand) {
                    const std::string cmd = readSeq(streamHandle);
                    getCommand = parseCommand(cmd);
                }
            }

            step();
        } catch (ExecExit& e) {
            std::ostringstream oss;
            oss << "\n" << e.what() << std::endl;
            streamHandle.putStr(oss.str());
            return e.code();
        }
    }
}


bool DebugInterpreter::parseCommand(std::string cmd) {}
