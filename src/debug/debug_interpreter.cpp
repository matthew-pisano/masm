//
// Created by matthew on 6/15/25.
//

#include "debug/debug_interpreter.h"

#include <sstream>

#include "exceptions.h"

/**
 * The prompt string for the debugger, displayed before each command input
 */
const std::string prompt = "\n(mdb) ";

/**
 * A help message for the debugger, listing available commands and their descriptions
 */
const std::string debuggerHelp =
        "Debug Interpreter Commands:\n\n"
        "break, b <address> - Set a breakpoint at the specified hex address\n"
        "break, b <line> - Set a breakpoint at the specified line number within the current file\n"
        "break, b <file:line> - Set a breakpoint at the specified file and line number\n"
        "continue, cont, c - Continue execution until the next breakpoint\n"
        "delete, d - Delete all breakpoints\n"
        "delete, d <num> - Delete the breakpoint with the specified number\n"
        "examine, x <address> - Examine memory at the specified address\n"
        "exit, quit, q - Exit the debugger\n"
        "finish - Execute until the end of the current procedure (the location stored in $ra)\n"
        "frame, f - Show the current stack frame\n"
        "help, h - Show this help message\n"
        "info, i breakpoints - List all breakpoints\n"
        "info, i registers - List all registers and their values\n"
        "info, i cp0 - List all Co-Processor 0 registers and their values\n"
        "info, i cp1 - List all Co-Processor 1 registers and their values\n"
        "list, ls, l - List the lines surrounding the current instruction\n"
        "next, n - Execute the next instruction, skipping over procedure calls\n"
        "print, p <$register> - Print the value of the specified register\n"
        "print, p <label> - Print the value of the specified label\n"
        "run, r - Run the program from the beginning until the next breakpoint or end of program\n"
        "step, s - Execute the next instruction\n";


State& DebugInterpreter::getState() { return state; }

void DebugInterpreter::setInteractive(const bool interactive) { isInteractive = interactive; }

int DebugInterpreter::interpret(const MemLayout& layout) {
    initProgram(layout);

    // Set initial breakpoint at start of program
    breakpoints[state.registers[Register::PC]] = 0;

    while (true) {
        try {
            // Skip interactive debug command input if not in interactive mode
            if (isInteractive) {
                uint32_t pc = state.registers[Register::PC];
                // Always get command if system breakpoint is zero
                bool getCommand = breakpoints.contains(pc) || breakpoints[0] == 0;

                // Clear system breakpoint
                for (const auto& [addr, id] : breakpoints)
                    if (id == 0) {
                        // Clear system breakpoint
                        breakpoints.erase(addr);
                        break;
                    }
                // Get user commands until none are expected
                while (getCommand) {
                    streamHandle.putStr(prompt);
                    const std::string cmdStr = readSeq(streamHandle);
                    getCommand = parseCommand(cmdStr, layout);
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


bool DebugInterpreter::parseCommand(const std::string& cmdStr, const MemLayout& layout) {
    std::vector<std::string> args;
    std::istringstream iss(cmdStr);
    std::string arg;
    while (iss >> arg)
        args.push_back(arg);

    const std::string cmd = args[0];
    if (cmd == "run" || cmd == "r") {
        // Clear state
        state = State(state.memory.isLittleEndian());
        // Clear Syscall State
        sysHandle = SystemHandle();
        // Reinitialize program with the current memory layout
        initProgram(layout);
        // Set initial breakpoint at start of program
        breakpoints[state.registers[Register::PC]] = 0;
        return false;
    }
    if (cmd == "help" || cmd == "h") {
        // Display help message with available commands
        streamHandle.putStr(debuggerHelp);
        return true;
    }
    if (cmd == "step" || cmd == "s") {
        // Signals that execution should stop after next executed instruction (not next in sequence)
        breakpoints[0] = 0;
        return false;
    }
    if (cmd == "next" || cmd == "n") {
        // Set breakpoint to be next instruction in sequence, skips over jumps
        breakpoints[state.registers[Register::PC] + 4] = 0;
        return false;
    }
    if (cmd == "continue" || cmd == "cont" || cmd == "c")
        // Continue execution until the next breakpoint
        return false;

    if (cmd == "break" || cmd == "b") {
        // Set a breakpoint at the specified address or line number
        if (args.size() < 2)
            streamHandle.putStr("Break requires an argument\n");
        else
            setBreakpoint(args[1]);
        return true;
    }
    if (cmd == "delete" || cmd == "d") {
        // Delete breakpoints
        deleteBreakpoint(args.size() < 2 ? "" : args[1]);
        return true;
    }
    if (cmd == "list" || cmd == "ls" || cmd == "l") {
        // List surrounding lines of code
        const uint32_t pc = state.registers[Register::PC];
        for (uint32_t i = pc - 40; i < pc + 40; i += 4) {
            if (!state.memory.isValid(i) || !state.debugInfo.contains(i))
                continue; // Skip invalid addresses
            const SourceLocator src = state.getDebugInfo(i);

            streamHandle.putStr(i == pc ? "--> " : "    ");
            streamHandle.putStr(std::format("{:<6} (0x{:08x}): 0x{:08x}\n", src.lineno, i,
                                            state.memory.wordAt(i)));
        }
        return true;
    }
    if (cmd == "frame" || cmd == "f") {
        // Show current stack frame
        const uint32_t fp = state.registers[Register::FP];
        const uint32_t sp = state.registers[Register::SP];
        for (uint32_t i = fp; i <= sp; i -= 4)
            streamHandle.putStr(std::format("0x{:08x}: 0x{:08x}\n", i, state.memory.wordAt(i)));
        return true;
    }
    if (cmd == "finish") {
        // Execute until the end of the current procedure
        breakpoints[state.registers[Register::RA]] = 0;
        return false;
    }
    if (cmd == "info" || cmd == "i") {
        // Show information about registers, breakpoints, etc.
        if (args[1] == "breakpoints")
            listBreakpoints();
        else if (args[1] == "registers")
            listRegisters();
        else if (args[1] == "cp0")
            listCP0Registers();
        else if (args[1] == "cp1")
            listCP1Registers();
        else
            streamHandle.putStr("Unknown info command: " + args[1] + "\n");
        return true;
    }
    if (cmd == "examine" || cmd == "x") {
        // Examine memory at the specified address
        if (args.size() < 2) {
            streamHandle.putStr("Examine requires an address argument\n");
            return true;
        }
        examineAddress(args[1]);
        return true;
    }
    if (cmd == "print" || cmd == "p") {
        // Print register or label value
        if (args.size() < 2) {
            streamHandle.putStr("Print requires a register or label argument\n");
            return true;
        }

        if (args[1].starts_with("$"))
            // Print register value
            printRegister(args[1].substr(1)); // Remove '$' prefix
        else
            // Print label value
            printLabel(args[1]);
        return true;
    }
    if (cmd == "exit" || cmd == "quit" || cmd == "q") {
        throw ExecExit("Exiting debugger", 0);
    }
    streamHandle.putStr("Unknown command: " + cmd + "\n");
    return true;
}

void DebugInterpreter::setBreakpoint(const std::string& arg) {
    // Check if the argument is a valid hex address
    if (arg.starts_with("0x")) {
        try {
            const uint32_t addr = std::stoul(arg.substr(2), nullptr, 16);
            breakpoints[addr] = nextBreakpoint;
            nextBreakpoint++;
            return;
        } catch (const std::invalid_argument&) {
            streamHandle.putStr("Invalid address: " + arg + "\n");
            return;
        }
    }

    size_t breakLine;
    std::string breakFile;

    if (arg.contains(":")) {
        // Check if the argument is a file:line format
        const size_t colonPos = arg.find(':');
        if (colonPos == std::string::npos) {
            streamHandle.putStr("Invalid file:line format: " + arg + "\n");
            return;
        }
        breakFile = arg.substr(0, colonPos);
        try {
            breakLine = std::stoul(arg.substr(colonPos + 1));
        } catch (const std::invalid_argument&) {
            streamHandle.putStr("Invalid line number: " + arg.substr(colonPos + 1) + "\n");
            return;
        }
    } else {
        const uint32_t pc = state.registers[Register::PC];
        breakFile = state.getDebugInfo(pc).filename;
        try {
            breakLine = std::stoul(arg);
        } catch (const std::invalid_argument&) {
            streamHandle.putStr("Invalid line number: " + arg + "\n");
            return;
        }
    }

    // Find debug info that matches file and line
    const auto it =
            std::ranges::find_if(state.debugInfo, [&breakFile, breakLine](const auto& pair) {
                return pair.second->filename == breakFile && pair.second->lineno == breakLine;
            });
    if (it == state.debugInfo.end())
        streamHandle.putStr("Cannot find line " + std::to_string(breakLine) + " in file " +
                            breakFile + "\n");
    else {
        const uint32_t addr = it->first;
        // Set breakpoint at the found address
        breakpoints[addr] = nextBreakpoint;
        nextBreakpoint++;
    }
}

void DebugInterpreter::deleteBreakpoint(const std::string& arg) {
    // Delete breakpoints
    if (arg.empty()) {
        for (const auto& [addr, id] : breakpoints) {
            // Do not delete the system breakpoint
            if (id == 0)
                continue;
            breakpoints.erase(addr);
        }
        return;
    }

    size_t breakpointId = stoui32(arg);
    if (breakpointId == 0) {
        streamHandle.putStr("No breakpoint found with ID 0\n");
        return;
    }
    // Find and delete the breakpoint with the specified ID
    const auto it = std::ranges::find_if(
            breakpoints, [breakpointId](const auto& pair) { return pair.second == breakpointId; });
    if (it != breakpoints.end())
        breakpoints.erase(it);
    else
        streamHandle.putStr("No breakpoint found with ID " + std::to_string(breakpointId) + "\n");
}


void DebugInterpreter::examineAddress(const std::string& arg) {
    if (!arg.starts_with("0x")) {
        streamHandle.putStr("Address must be in hex format (0x...)\n");
        return;
    }

    try {
        const uint32_t addr = std::stoul(arg.substr(2), nullptr, 16);
        if (!state.memory.isValid(addr)) {
            streamHandle.putStr("Invalid address: " + arg + "\n");
            return;
        }
        const int32_t value = state.memory.wordAt(addr);
        streamHandle.putStr(std::format("0x{:08x}: 0x{:08x}\n", addr, value));
    } catch (const std::invalid_argument&) {
        streamHandle.putStr("Invalid address format: " + arg + "\n");
    }
}

void DebugInterpreter::listBreakpoints() {}

void DebugInterpreter::listRegisters() {}

void DebugInterpreter::listCP0Registers() {}

void DebugInterpreter::listCP1Registers() {}

void DebugInterpreter::printRegister(const std::string& arg) {}

void DebugInterpreter::printLabel(const std::string& arg) {}

bool DebugInterpreter::validateArguments(const std::string& cmd,
                                         const std::vector<std::string>& args) {
    return true;
}
