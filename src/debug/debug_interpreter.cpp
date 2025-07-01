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
        "break, b <ref> - Set a breakpoint at the given reference.  This can be in the form of a "
        "hexadecimal address, a line number, a label, or a filename:line or filename:label pair\n"
        "continue, cont, c - Continue execution until the next breakpoint\n"
        "delete, d - Delete all breakpoints\n"
        "delete, d <num> - Delete the breakpoint with the specified number\n"
        "examine, x <ref> [words] - Examine memory at the given reference.  This can be in the "
        "form of a hexadecimal address, a line number, a label, or a filename:line or "
        "filename:label pair.  The number of words to print can also be specified; one by default\n"
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
        "print, p <ref> - Print the string value of the specified location reference\n"
        "run, r - Run the program from the beginning until the next breakpoint or end of program\n"
        "step, s - Execute the next instruction\n";


/**
 * Converts a byte into a single printable character
 * @param byte The byte to convert
 * @return A printable representation of the byte, with non-printable characters replaced by '.'
 */
char byteAsString(const int8_t byte) {
    if (byte >= 32 && byte < 127)
        return byte;
    return '.';
}


/**
 * Converts a word into a 4 character string by translating the bytes
 * @param word The word to convert
 * @return A string representation of the word, with each byte represented as a character
 */
std::string wordAsString(const uint32_t word) {
    std::string result;
    for (int i = 0; i < 4; ++i) {
        const char c = static_cast<char>(word >> (i * 8) & 0xFF);
        result += byteAsString(c);
    }
    return result;
}


std::string DebugInterpreter::strAt(const uint32_t addr, const size_t maxLen) {
    std::string result;
    while (result.length() < maxLen) {
        const uint8_t byte = state.memory.byteAt(addr + result.length());
        if (byte == 0)
            break; // Stop at null terminator
        result += byteAsString(static_cast<int8_t>(byte));
    }
    return result;
}

std::string DebugInterpreter::strAt(const uint32_t addr) {
    return strAt(addr, std::numeric_limits<size_t>::max());
}


State& DebugInterpreter::getState() { return state; }

void DebugInterpreter::setInteractive(const bool interactive) { isInteractive = interactive; }

void DebugInterpreter::interactiveStep(const MemLayout& layout) {
    const uint32_t pc = state.registers[Register::PC];
    // Always get command if system breakpoint is zero
    bool getCommand = breakpoints.contains(pc) || (breakpoints.contains(0) && breakpoints[0] == 0);
    // If execution has reached the system breakpoint
    const bool atSystemBreakpoint = (breakpoints.contains(pc) && breakpoints[pc] == 0) ||
                                    (breakpoints.contains(0) && breakpoints[0] == 0);

    if (atSystemBreakpoint) {
        // Clear system breakpoint
        const auto it = std::ranges::find_if(breakpoints,
                                             [](const auto& pair) { return pair.second == 0; });
        if (it != breakpoints.end())
            breakpoints.erase(it);
    }
    // Get user commands until none are expected
    while (getCommand) {
        streamHandle.putStr(prompt);
        const std::string cmdStr = readSeq(streamHandle);
        getCommand = execCommand(cmdStr, layout);
    }
}

int DebugInterpreter::interpret(const MemLayout& layout) {
    initProgram(layout);
    // Set initial breakpoint at start of program
    breakpoints[state.registers[Register::PC]] = 0;
    isRunning = true;

    while (true) {
        try {
            // Skip interactive debug command input if not in interactive mode
            if (isInteractive)
                interactiveStep(layout);

            if (!isRunning) {
                streamHandle.putStr("\nThere is no program running.  Use 'run' to restart\n");
                // Set system breakpoint to PC
                breakpoints[state.registers[Register::PC]] = 0;
            } else
                step();
        } catch (DebuggerExit& e) {
            streamHandle.putStr(std::format("\n{}", e.what()));
            return e.code();
        } catch (MasmRuntimeError& e) {
            if (!isInteractive)
                throw;
            streamHandle.putStr(std::format("\n{}", e.what()));
            isRunning = false;
            // Decrement PC to offending instruction
            state.registers[Register::PC] -= 4;
        } catch (ExecExit& e) {
            streamHandle.putStr(std::format("\n{}", e.what()));
            isRunning = false;
            if (!isInteractive)
                return e.code();

            // Decrement PC
            state.registers[Register::PC] -= 4;
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
        if (!args.empty())
            throw std::invalid_argument("Run command does not take any arguments");
        return {DebugCommand::RUN, args};
    }
    if (cmd == "help" || cmd == "h") {
        if (!args.empty())
            throw std::invalid_argument("Help command does not take any arguments");
        return {DebugCommand::HELP, args};
    }
    if (cmd == "step" || cmd == "s") {
        if (!args.empty())
            throw std::invalid_argument("Step command does not take any arguments");
        return {DebugCommand::STEP, args};
    }
    if (cmd == "next" || cmd == "n") {
        if (!args.empty())
            throw std::invalid_argument("Next command does not take any arguments");
        return {DebugCommand::NEXT, args};
    }
    if (cmd == "continue" || cmd == "cont" || cmd == "c") {
        if (!args.empty())
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
        return {DebugCommand::DEL_BP, {args}};
    }
    if (cmd == "list" || cmd == "ls" || cmd == "l") {
        if (args.size() > 1)
            throw std::invalid_argument("List command requires zero or one argument");
        return {DebugCommand::LIST, args};
    }
    if (cmd == "frame" || cmd == "f") {
        if (!args.empty())
            throw std::invalid_argument("Frame command does not take any arguments");
        return {DebugCommand::FRAME, args};
    }
    if (cmd == "finish") {
        if (!args.empty())
            throw std::invalid_argument("Finish command does not take any arguments");
        return {DebugCommand::FINISH, args};
    }
    if (cmd == "info" || cmd == "i") {
        if (args.size() != 1)
            throw std::invalid_argument("Info command requires one argument");
        return {DebugCommand::INFO, args};
    }
    if (cmd == "examine" || cmd == "x") {
        if (args.empty() || args.size() > 2)
            throw std::invalid_argument("Examine command requires one or two arguments");
        return {DebugCommand::EXAMINE, args};
    }
    if (cmd == "print" || cmd == "p") {
        if (args.size() != 1)
            throw std::invalid_argument("Print command requires one argument");
        return {DebugCommand::PRINT, args};
    }
    if (cmd == "exit" || cmd == "quit" || cmd == "q") {
        if (!args.empty())
            throw std::invalid_argument("Exit command does not take any arguments");
        return {DebugCommand::EXIT, args};
    }

    throw std::invalid_argument("Unknown debug command: " + cmd);
}

bool DebugInterpreter::execCommand(const std::string& cmdStr, const MemLayout& layout) {
    // Parse the command string into a command and its arguments
    try {
        const std::tuple<DebugCommand, std::vector<std::string>> parsedCmd = parseCommand(cmdStr);
        const DebugCommand cmd = std::get<0>(parsedCmd);
        const std::vector<std::string> args = std::get<1>(parsedCmd);

        switch (cmd) {
            case DebugCommand::RUN: {
                // Reset the interpreter and run the program from the beginning
                resetInterpreter(layout);
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
                // Set a breakpoint at the specified address, line number, or label
                setBreakpoint(args[0]);
                return true;
            }
            case DebugCommand::DEL_BP: {
                // Delete breakpoints
                deleteBreakpoint(args.empty() ? "" : args[0]);
                return true;
            }
            case DebugCommand::LIST: {
                // List surrounding lines of code
                listLines(args.empty() ? "" : args[0]);
                return true;
            }
            case DebugCommand::FRAME: {
                // Show current stack frame
                getFrame();
                return true;
            }
            case DebugCommand::FINISH: {
                // Execute until the end of the current procedure
                if (state.registers[Register::RA] != 0)
                    breakpoints[state.registers[Register::RA]] = 0;
                else if (state.cp0[Coproc0Register::EPC] != 0)
                    breakpoints[state.cp0[Coproc0Register::EPC]] = 0;
                else {
                    streamHandle.putStr("No return address found to finish execution\n");
                    return true;
                }
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
                size_t numWords = 1;
                if (args.size() > 1)
                    numWords = stoui32(args[1]);

                examineAddress(args[0], numWords);
                return true;
            }
            case DebugCommand::PRINT: {
                // Print register or reference value
                if (args[0].starts_with("$"))
                    // Print register value
                    printRegister(args[0].substr(1)); // Remove '$' prefix
                else
                    // Print reference value
                    printRef(args[0]);
                return true;
            }
            case DebugCommand::EXIT: {
                throw DebuggerExit("Exiting debugger", 0);
            }
        }
    } catch (const DebuggerExit& e) {
        throw;
    } catch (const std::exception& e) {
        streamHandle.putStr(std::format("\n{}", e.what()));
        return true; // Continue prompting for commands
    }
    return false;
}

void DebugInterpreter::resetInterpreter(const MemLayout& layout) {
    // Clear state
    state = State(state.memory.isLittleEndian());
    // Clear Syscall State
    sysHandle = SystemHandle();
    // Reinitialize program with the current memory layout
    initProgram(layout);
    // Set initial breakpoint at start of program
    breakpoints[state.registers[Register::PC]] = 0;
    isRunning = true;
}

void DebugInterpreter::listLines(const std::string& arg) {
    const uint32_t pc = state.registers[Register::PC];
    const uint32_t addr = arg.empty() ? pc : addrFRomStr(arg);

    for (uint32_t i = addr - 40; i < addr + 40; i += 4) {
        if (!state.memory.isValid(i) || !state.debugInfo.contains(i))
            continue; // Skip invalid addresses

        const DebugInfo debugInfo = state.getDebugInfo(i);
        if (!debugInfo.label.empty())
            streamHandle.putStr("(" + unmangleLabel(debugInfo.label) + ")\n");
        const std::string pointerString = i == pc ? "--->" : "";
        // An indicator for when a breakpoint is present
        const std::string bpString =
                breakpoints.contains(i) ? "(*" + std::to_string(breakpoints[i]) + ")" : "";
        streamHandle.putStr(std::format("{:<6} {:<4} {:<6} (0x{:08x}): 0x{:08x}    {}\n", bpString,
                                        pointerString, debugInfo.source.lineno, i,
                                        static_cast<uint32_t>(state.memory.wordAt(i)),
                                        debugInfo.source.text));
    }
}

void DebugInterpreter::getFrame() {
    const uint32_t fp = state.registers[Register::FP];
    const uint32_t sp = state.registers[Register::SP];
    for (uint32_t i = fp; i >= sp; i -= 4)
        streamHandle.putStr(std::format("0x{:08x}: 0x{:08x}\n", i, state.memory.wordAt(i)));
}

size_t DebugInterpreter::locateLabelInFile(const std::string& label, const std::string& filename) {
    // Find debug info that matches the given label in the current file
    const auto it = std::ranges::find_if(state.debugInfo, [label, filename](const auto& pair) {
        return unmangleLabel(pair.second.label) == label && pair.second.source.filename == filename;
    });
    if (it == state.debugInfo.end()) {
        throw std::invalid_argument("Cannot find label: '" + label + "' in file " + filename +
                                    "\n");
    }
    // Get the line for the label
    return it->second.source.lineno;
}

uint32_t DebugInterpreter::addrFRomStr(const std::string& ref) {
    // Check if the argument is a valid hex address
    if (ref.starts_with("0x")) {
        try {
            return std::stoul(ref.substr(2), nullptr, 16);
        } catch (const std::invalid_argument&) {
            throw std::invalid_argument("Invalid hexadecimal address: " + ref);
        }
    }

    size_t refLine;
    // Current file, by default
    std::string refFile = state.getDebugInfo(state.registers[Register::PC]).source.filename;
    // If the argument is in file:line or file:label format
    if (ref.contains(":")) {
        const size_t colonPos = ref.find(':');
        refFile = ref.substr(0, colonPos);
        try {
            refLine = std::stoul(ref.substr(colonPos + 1));
        } catch (const std::invalid_argument&) {
            // Treat portion after colon as a label
            refLine = locateLabelInFile(ref.substr(colonPos + 1), refFile);
        }
    }
    // If the argument is just a line number or label
    else {
        try {
            refLine = std::stoul(ref);
        } catch (const std::invalid_argument&) {
            // Treat the reference as a label
            refLine = locateLabelInFile(ref, refFile);
        }
    }

    // Find debug info that matches file and line
    const auto it = std::ranges::find_if(state.debugInfo, [refLine, refFile](const auto& pair) {
        const SourceLocator src = pair.second.source;
        return src.filename == refFile && src.lineno == refLine;
    });
    if (it == state.debugInfo.end())
        throw std::invalid_argument("Cannot find memory at " + refFile + ":" +
                                    std::to_string(refLine) + "\n");
    return it->first;
}

void DebugInterpreter::setBreakpoint(const std::string& arg) {
    const uint32_t addr = addrFRomStr(arg);
    if (!breakpoints.contains(addr)) {
        // Set breakpoint at the found address
        breakpoints[addr] = nextBreakpoint;
        nextBreakpoint++;
        streamHandle.putStr(
                std::format("Breakpoint {} set at 0x{:08x}\n", breakpoints[addr], addr));
    } else
        streamHandle.putStr(
                std::format("Breakpoint {} already exists at 0x{:08x}\n", breakpoints[addr], addr));
}

void DebugInterpreter::deleteBreakpoint(const std::string& arg) {
    // Delete all breakpoints
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


void DebugInterpreter::examineAddress(const std::string& arg, const size_t numWords) {
    const uint32_t addr = addrFRomStr(arg);

    for (size_t i = 0; i < numWords; i++) {
        uint32_t value = state.memory._sysWordAt(addr + i * 4);
        streamHandle.putStr(
                std::format("0x{:08x}: 0x{:08x} ({})\n", addr + i * 4, value, wordAsString(value)));
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
        SourceLocator src = state.getDebugInfo(addr).source;
        streamHandle.putStr(
                std::format("{:<3}: 0x{:08x} ({}:{})\n", id, addr, src.filename, src.lineno));
    }
}

void DebugInterpreter::listLabels() {
    for (const auto& [addr, debugInfo] : state.debugInfo)
        if (!debugInfo.label.empty()) {
            const SourceLocator src = debugInfo.source;
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
            streamHandle.putStr(std::format("${} -> 0x{:08x}\n", arg, value));
        } catch (const std::exception&) {
            streamHandle.putStr("Invalid register: " + arg + " (CPU registers expect an alias)\n");
        }
    }
}

void DebugInterpreter::printRef(const std::string& arg) {
    const uint32_t addr = addrFRomStr(arg);

    if (state.debugInfo.contains(addr)) {
        const DebugInfo debugInfo = state.debugInfo[addr];
        streamHandle.putStr(std::format("({}:{}) -> \"{}\" \n", debugInfo.source.filename,
                                        debugInfo.source.lineno, strAt(addr)));
    } else
        streamHandle.putStr(std::format("\"{}\" \n", strAt(addr)));
}
