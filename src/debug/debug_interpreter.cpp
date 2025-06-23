//
// Created by matthew on 6/15/25.
//

#include "debug/debug_interpreter.h"

#include <algorithm>
#include <sstream>

#include "exceptions.h"
#include "tokenizer/postprocessor.h"

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
        "info, i labels - List all labels\n"
        "info, i registers - List all registers and their values\n"
        "info, i cp0 - List all Co-Processor 0 registers and their values\n"
        "info, i cp1 - List all Co-Processor 1 registers and their values\n"
        "list, ls, l - List the lines surrounding the current instruction\n"
        "next, n - Execute the next instruction, skipping over procedure calls\n"
        "print, p <$register> - Print the value of the specified register\n"
        "print, p <label> - Print the value of the specified label\n"
        "run, r - Run the program from the beginning until the next breakpoint or end of program\n"
        "step, s - Execute the next instruction\n";


/**
 * Converts a word into a 4 character string by translating the bytes
 * @param word The word to convert
 * @return A string representation of the word, with each byte represented as a character
 */
std::string wordAsString(const uint32_t word) {
    std::string result;
    for (int i = 0; i < 4; ++i) {
        const char c = static_cast<char>(word >> (i * 8) & 0xFF);
        if (c >= 32 && c < 127)
            result += c;
        else
            result += '.';
    }
    return result;
}


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
                bool getCommand = breakpoints.contains(pc) ||
                                  (breakpoints.contains(0) && breakpoints[0] == 0);
                // If execution has reached the system breakpoint
                const bool atSystemBreakpoint =
                        (breakpoints.contains(pc) && breakpoints[pc] == 0) ||
                        (breakpoints.contains(0) && breakpoints[0] == 0);

                if (atSystemBreakpoint)
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
                    getCommand = execCommand(cmdStr, layout);
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

std::tuple<DebugCommand, std::vector<std::string>>
DebugInterpreter::parseCommand(const std::string& cmdStr) {
    if (cmdStr.empty())
        throw std::invalid_argument("Command cannot be empty");

    std::vector<std::string> args;
    std::istringstream iss(cmdStr);
    std::string arg;
    while (iss >> arg)
        args.push_back(arg);

    const std::string cmd = args[0];
    // Remove the command from the arguments
    args.erase(args.begin());
    if (cmd == "run" || cmd == "r") {
        if (args.size() > 0)
            throw std::invalid_argument("Run command does not take any arguments");
        return {DebugCommand::RUN, args};
    }
    if (cmd == "help" || cmd == "h") {
        if (args.size() > 0)
            throw std::invalid_argument("Help command does not take any arguments");
        return {DebugCommand::HELP, args};
    }
    if (cmd == "step" || cmd == "s") {
        if (args.size() > 0)
            throw std::invalid_argument("Step command does not take any arguments");
        return {DebugCommand::STEP, args};
    }
    if (cmd == "next" || cmd == "n") {
        if (args.size() > 0)
            throw std::invalid_argument("Next command does not take any arguments");
        return {DebugCommand::NEXT, args};
    }
    if (cmd == "continue" || cmd == "cont" || cmd == "c") {
        if (args.size() > 0)
            throw std::invalid_argument("Continue command does not take any arguments");
        return {DebugCommand::CONTINUE, args};
    }
    if (cmd == "break" || cmd == "b") {
        if (args.size() != 1)
            throw std::invalid_argument("Break command requires one argument");
        return {DebugCommand::BREAK, args};
    }
    if (cmd == "delete" || cmd == "d") {
        if (args.size() > 1)
            throw std::invalid_argument("Delete command requires zero or one argument");
        return {DebugCommand::DELETE, {args}};
    }
    if (cmd == "list" || cmd == "ls" || cmd == "l") {
        if (args.size() > 0)
            throw std::invalid_argument("List command does not take any arguments");
        return {DebugCommand::LIST, args};
    }
    if (cmd == "frame" || cmd == "f") {
        if (args.size() > 0)
            throw std::invalid_argument("Frame command does not take any arguments");
        return {DebugCommand::FRAME, args};
    }
    if (cmd == "finish") {
        if (args.size() > 0)
            throw std::invalid_argument("Finish command does not take any arguments");
        return {DebugCommand::FINISH, args};
    }
    if (cmd == "info" || cmd == "i") {
        if (args.size() != 1)
            throw std::invalid_argument("Info command requires one argument");
        return {DebugCommand::INFO, args};
    }
    if (cmd == "examine" || cmd == "x") {
        if (args.size() != 1)
            throw std::invalid_argument("Examine command requires one argument");
        return {DebugCommand::EXAMINE, args};
    }
    if (cmd == "print" || cmd == "p") {
        if (args.size() != 1)
            throw std::invalid_argument("Print command requires one argument");
        return {DebugCommand::PRINT, args};
    }
    if (cmd == "exit" || cmd == "quit" || cmd == "q") {
        if (args.size() > 0)
            throw std::invalid_argument("Exit command does not take any arguments");
        return {DebugCommand::EXIT, args};
    }

    throw std::invalid_argument("Unknown debug command: " + cmd);
}

bool DebugInterpreter::execCommand(const std::string& cmdStr, const MemLayout& layout) {
    // Parse the command string into a command and its arguments
    DebugCommand cmd;
    std::vector<std::string> args;
    try {
        const std::tuple<DebugCommand, std::vector<std::string>> parsedCmd = parseCommand(cmdStr);
        cmd = std::get<0>(parsedCmd);
        args = std::get<1>(parsedCmd);
    } catch (const std::invalid_argument& e) {
        std::ostringstream oss;
        oss << "\n" << e.what() << std::endl;
        streamHandle.putStr(oss.str());
        return true; // Continue prompting for commands
    }

    switch (cmd) {
        case DebugCommand::RUN: {
            // Clear state
            state = State(state.memory.isLittleEndian());
            // Clear Syscall State
            sysHandle = SystemHandle();
            // Reinitialize program with the current memory layout
            initProgram(layout);
            // Set initial breakpoint at start of program
            breakpoints[state.registers[Register::PC]] = 0;
            return true;
        }
        case DebugCommand::HELP: {
            // Display help message with available commands
            streamHandle.putStr(debuggerHelp);
            return true;
        }
        case DebugCommand::STEP: {
            // Signals that execution should stop after next executed instruction (not next in
            // sequence)
            breakpoints[0] = 0;
            return false;
        }
        case DebugCommand::NEXT: {
            // Set breakpoint to be next instruction in sequence, skips over jumps
            breakpoints[state.registers[Register::PC] + 4] = 0;
            return false;
        }
        case DebugCommand::CONTINUE:
            // Continue execution until the next breakpoint
            return false;
        case DebugCommand::BREAK: {
            // Set a breakpoint at the specified address or line number
            setBreakpoint(args[0]);
            return true;
        }
        case DebugCommand::DELETE: {
            // Delete breakpoints
            deleteBreakpoint(args.empty() ? "" : args[0]);
            return true;
        }
        case DebugCommand::LIST: {
            // List surrounding lines of code
            const uint32_t pc = state.registers[Register::PC];
            for (uint32_t i = pc - 40; i < pc + 40; i += 4) {
                if (!state.memory.isValid(i) || !state.debugInfo.contains(i))
                    continue; // Skip invalid addresses

                const DebugInfo debugInfo = state.getDebugInfo(i);
                if (!debugInfo.label.empty())
                    streamHandle.putStr("(" + unmangleLabel(debugInfo.label) + ")\n");
                streamHandle.putStr(i == pc ? "--> " : "    ");
                streamHandle.putStr(std::format("{:<6} (0x{:08x}): 0x{:08x}\n",
                                                debugInfo.source->lineno, i,
                                                state.memory.wordAt(i)));
            }
            return true;
        }
        case DebugCommand::FRAME: {
            // Show current stack frame
            const uint32_t fp = state.registers[Register::FP];
            const uint32_t sp = state.registers[Register::SP];
            for (uint32_t i = fp; i >= sp; i -= 4)
                streamHandle.putStr(std::format("0x{:08x}: 0x{:08x}\n", i, state.memory.wordAt(i)));
            return true;
        }
        case DebugCommand::FINISH: {
            // Execute until the end of the current procedure
            if (state.registers[Register::RA] != 0)
                breakpoints[state.registers[Register::RA]] = 0;
            return false;
        }
        case DebugCommand::INFO: {
            // Show information about registers, breakpoints, etc.
            if (args[0] == "breakpoints")
                listBreakpoints();
            else if (args[0] == "labels")
                listLabels();
            else if (args[0] == "registers")
                listRegisters();
            else if (args[0] == "cp0")
                listCP0Registers();
            else if (args[0] == "cp1")
                listCP1Registers();
            else
                streamHandle.putStr("Unknown info command: " + args[0] + "\n");
            return true;
        }
        case DebugCommand::EXAMINE: {
            // Examine memory at the specified address
            examineAddress(args[0]);
            return true;
        }
        case DebugCommand::PRINT: {
            // Print register or label value
            if (args[0].starts_with("$"))
                // Print register value
                printRegister(args[0].substr(1)); // Remove '$' prefix
            else
                // Print label value
                printLabel(args[0]);
            return true;
        }
        case DebugCommand::EXIT: {
            throw ExecExit("Exiting debugger", 0);
        }
    }
    // Should never be reached
    return false;
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
        breakFile = state.getDebugInfo(pc).source->filename;
        try {
            breakLine = std::stoul(arg);
        } catch (const std::invalid_argument&) {
            streamHandle.putStr("Invalid line number: " + arg + "\n");
            return;
        }
    }

    // Find debug info that matches file and line
    const auto it = std::ranges::find_if(state.debugInfo, [breakFile, breakLine](const auto& pair) {
        const SourceLocator src = *pair.second.source;
        return src.filename == breakFile && src.lineno == breakLine;
    });
    if (it == state.debugInfo.end())
        streamHandle.putStr("Cannot find line " + std::to_string(breakLine) + " in file " +
                            breakFile + "\n");
    else {
        const uint32_t addr = it->first;
        if (!breakpoints.contains(addr)) {
            // Set breakpoint at the found address
            breakpoints[addr] = nextBreakpoint;
            nextBreakpoint++;
        } else
            streamHandle.putStr(std::format("Breakpoint {} already exists at 0x{:08x}\n",
                                            breakpoints[addr], addr));
    }
}

void DebugInterpreter::deleteBreakpoint(const std::string& arg) {
    // Delete breakpoints
    if (arg.empty()) {
        std::vector<uint32_t> breakAddrs;
        for (auto it = breakpoints.begin(); it != breakpoints.end();) {
            if (it->second == 0)
                ++it;
            else
                it = breakpoints.erase(it);
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
        streamHandle.putStr(
                std::format("0x{:08x}: 0x{:08x} ({})\n", addr, value, wordAsString(value)));
    } catch (const std::invalid_argument&) {
        streamHandle.putStr("Invalid address format: " + arg + "\n");
    }
}

void DebugInterpreter::listBreakpoints() {
    if (breakpoints.empty()) {
        streamHandle.putStr("No breakpoints set.\n");
        return;
    }

    for (const auto& [addr, id] : breakpoints) {
        if (id == 0)
            continue; // Skip system breakpoint
        const SourceLocator src = *state.getDebugInfo(addr).source;
        streamHandle.putStr(
                std::format("{:<3}: 0x{:08x} ({}:{})\n", id, addr, src.filename, src.lineno));
    }
}

void DebugInterpreter::listLabels() {
    for (const auto& [addr, debugInfo] : state.debugInfo)
        if (!debugInfo.label.empty() && debugInfo.source) {
            const SourceLocator src = *debugInfo.source;
            streamHandle.putStr(std::format("{} -> 0x{:08x} ({}:{})\n",
                                            unmangleLabel(debugInfo.label), addr, src.filename,
                                            src.lineno));
        } else if (!debugInfo.label.empty())
            streamHandle.putStr(
                    std::format("{} -> 0x{:08x}\n", unmangleLabel(debugInfo.label), addr));
}

void DebugInterpreter::listRegisters() {
    for (size_t i = 0; i < NUM_CPU_REGISTERS; ++i) {
        const int32_t value = state.registers[i];
        streamHandle.putStr(
                std::format("${:<5}: 0x{:08x}\n", RegisterFile::nameFromIndex(i), value));
    }
}

void DebugInterpreter::listCP0Registers() {
    const int32_t vaddrValue = state.cp0[Coproc0Register::VADDR];
    streamHandle.putStr(std::format("$8  : 0x{:08x}\n", vaddrValue));
    const int32_t statusValue = state.cp0[Coproc0Register::STATUS];
    streamHandle.putStr(std::format("$12 : 0x{:08x}\n", statusValue));
    const int32_t causeValue = state.cp0[Coproc0Register::CAUSE];
    streamHandle.putStr(std::format("$13 : 0x{:08x}\n", causeValue));
    const int32_t epcValue = state.cp0[Coproc0Register::EPC];
    streamHandle.putStr(std::format("$14 : 0x{:08x}\n", epcValue));
}

void DebugInterpreter::listCP1Registers() {
    for (size_t i = 0; i < NUM_CP1_REGISTERS; ++i) {
        const int32_t value = state.cp1[i];
        const float32_t floatValue = state.cp1.getFloat(i);
        if (i % 2 == 0) {
            const float64_t doubleValue = state.cp1.getDouble(i);
            streamHandle.putStr(std::format("${:<4}: 0x{:08x} ({:.6f}, {:.6f})\n",
                                            Coproc1RegisterFile::nameFromIndex(i), value,
                                            floatValue, doubleValue));
        } else
            streamHandle.putStr(std::format("${:<4}: 0x{:08x} ({:.6f})\n",
                                            Coproc1RegisterFile::nameFromIndex(i), value,
                                            floatValue));
    }
}

void DebugInterpreter::printRegister(const std::string& arg) {
    if (arg.size() > 1 && arg[0] == 'f') {
        // Co-Processor 1 register
        try {
            const size_t index = Coproc1RegisterFile::indexFromName(arg);
            int32_t value = state.cp1[index];
            streamHandle.putStr(std::format("$f{}: 0x{:08x}\n", index, value));
        } catch (const std::exception&) {
            streamHandle.putStr("Invalid Co-Processor 1 register: " + arg + "\n");
        }
    } else if (arg == "8" || arg == "12" || arg == "13" || arg == "14") {
        // Special Co-Processor 0 registers
        const size_t index = std::stoi(arg);
        int32_t value = state.cp0[index];
        streamHandle.putStr(std::format("${}: 0x{:08x}\n", index, value));
    } else {
        // General-purpose register
        try {
            size_t index;
            if (arg == "pc")
                index = static_cast<size_t>(Register::PC);
            else if (arg == "hi")
                index = static_cast<size_t>(Register::HI);
            else if (arg == "lo")
                index = static_cast<size_t>(Register::LO);
            else
                index = RegisterFile::indexFromName(arg);

            int32_t value = state.registers[index];
            streamHandle.putStr(std::format("${}: 0x{:08x}\n", arg, value));
        } catch (const std::exception&) {
            streamHandle.putStr("Invalid register: " + arg + " (CPU registers expect an alias)\n");
        }
    }
}

void DebugInterpreter::printLabel(const std::string& arg) {
    // Check if the label exists in the debug info
    const auto it = std::ranges::find_if(state.debugInfo, [&arg](const auto& pair) {
        return unmangleLabel(pair.second.label) == arg;
    });
    if (it != state.debugInfo.end()) {
        const uint32_t addr = it->first;
        const DebugInfo& debugInfo = it->second;
        if (debugInfo.source)
            streamHandle.putStr(std::format("{} -> 0x{:08x} ({}:{})\n", arg, addr,
                                            debugInfo.source->filename, debugInfo.source->lineno));
        else
            streamHandle.putStr(std::format("{} -> 0x{:08x}\n", arg, addr));
    } else {
        streamHandle.putStr("Label not found: " + arg + "\n");
    }
}
