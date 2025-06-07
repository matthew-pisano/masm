//
// Created by matthew on 4/24/25.
//

#include "interpreter/interpreter.h"

#include <sstream>
#include <stdexcept>

#include "exceptions.h"
#include "interpreter/syscalls.h"
#include "io/consoleio.h"
#include "parser/instruction.h"
#include "utils.h"


std::string causeToString(const uint32_t cause) {
    if (cause & static_cast<uint32_t>(INTERP_CODE::KEYBOARD_INTERP))
        return "MMIO read interrupt failed";
    if (cause & static_cast<uint32_t>(INTERP_CODE::DISPLAY_INTERP))
        return "MMIO write interrupt failed";

    const uint32_t excCode = cause & 0x007c; // Zero out all bits except for [2-6]
    if (excCode == static_cast<uint32_t>(EXCEPT_CODE::ADDRESS_EXCEPTION_LOAD))
        return "Failed to load address";
    if (excCode == static_cast<uint32_t>(EXCEPT_CODE::ADDRESS_EXCEPTION_STORE))
        return "Failed to store address";
    if (excCode == static_cast<uint32_t>(EXCEPT_CODE::SYSCALL_EXCEPTION))
        return "Failed to execute syscall";
    if (excCode == static_cast<uint32_t>(EXCEPT_CODE::BREAKPOINT_EXCEPTION))
        return "Failed to handle breakpoint";
    if (excCode == static_cast<uint32_t>(EXCEPT_CODE::RESERVED_INSTRUCTION_EXCEPTION))
        return "Attempted to execute reserved instruction";
    if (excCode == static_cast<uint32_t>(EXCEPT_CODE::ARITHMETIC_OVERFLOW_EXCEPTION))
        return "Integer overflow";
    if (excCode == static_cast<uint32_t>(EXCEPT_CODE::TRAP_EXCEPTION))
        return "Trap exception occurred";
    if (excCode == static_cast<uint32_t>(EXCEPT_CODE::DIVIDE_BY_ZERO_EXCEPTION))
        return "Division by zero";
    if (excCode == static_cast<uint32_t>(EXCEPT_CODE::FLOATING_POINT_OVERFLOW))
        return "Floating point overflow";
    if (excCode == static_cast<uint32_t>(EXCEPT_CODE::FLOATING_POINT_UNDERFLOW))
        return "Floating point underflow";

    return "Unknown exception code: " + std::to_string(excCode);
}


SourceLocator State::getDebugInfo(const uint32_t addr) const { return *debugInfo.at(addr); }


void State::loadProgram(const MemLayout& layout) {
    for (const std::pair<MemSection, std::vector<std::byte>> pair : layout.data)
        for (size_t i = 0; i < pair.second.size(); i++) {
            uint32_t memOffset = memSectionOffset(pair.first) + i;
            memory[memOffset] = pair.second[i];

            // Add debug info from layout if the address is executable text
            if (isSectionExecutable(pair.first)) {
                const std::shared_ptr<SourceLocator> sourceLine =
                        layout.debugInfo.at(pair.first).at(i);
                debugInfo[memOffset] = sourceLine;
            }
        }
}


void Interpreter::initProgram(const MemLayout& layout) {
    state.loadProgram(layout);
    // Initialize PC to the start of the text section
    state.registers[Register::PC] = static_cast<int32_t>(memSectionOffset(MemSection::TEXT));
    // Initialize the stack registers
    state.registers[Register::FP] = static_cast<int32_t>(memSectionOffset(MemSection::STACK));
    state.registers[Register::SP] = static_cast<int32_t>(memSectionOffset(MemSection::STACK));
    // Initialize the global pointer to the start of the global section
    state.registers[Register::GP] = static_cast<int32_t>(memSectionOffset(MemSection::GLOBAL));
    // Set MMIO output ready bit to 1
    state.memory[memSectionOffset(MemSection::MMIO) + 8 + 3] = std::byte{1};
    // Enable interrupts
    // Enable MMIO interrupt bits for keyboard and display
    state.cp0[Coproc0Register::STATUS] |= static_cast<int32_t>(INTERP_CODE::DISPLAY_INTERP) |
                                          static_cast<int32_t>(INTERP_CODE::KEYBOARD_INTERP);
}


int Interpreter::interpret(const MemLayout& layout) {
    initProgram(layout);

    while (true) {
        try {
            step();
        } catch (ExecExit& e) {
            ostream << "\n" << e.what() << std::endl;
            return e.code();
        }
    }
}


bool Interpreter::readMMIO() {
    if (ioMode != IOMode::MMIO)
        throw std::runtime_error("MMIO mode not enabled for reading input");

    const uint32_t input_ready = memSectionOffset(MemSection::MMIO);
    const uint32_t input_data = input_ready + 4;

    if (state.memory.wordAt(input_ready) != 0)
        // Return if the previous character has yet to be read by the program
        return false;

    char c = 0;
    // Check if the input stream has characters to read
    if (&istream == &std::cin && consoleHasChar())
        c = consoleGetChar();
    else if (istream.peek() != std::char_traits<char>::eof())
        istream.get(c);

    if (c) {
        // Set the ready bit
        state.memory[input_ready + 3] = std::byte{1};
        // Set the input data byte
        state.memory[input_data + 3] = static_cast<std::byte>(c);
    }

    // Clear error flags from peeking when stream is empty
    istream.clear();

    // Return true if a character was read, false otherwise
    return static_cast<bool>(c);
}


bool Interpreter::writeMMIO() {
    if (ioMode != IOMode::MMIO)
        throw std::runtime_error("MMIO mode not enabled for writing output");

    const uint32_t output_ready = memSectionOffset(MemSection::MMIO) + 8;
    const uint32_t output_data = output_ready + 4;

    // Check if the output stream is ready to write
    if (state.memory.wordAt(output_ready) != 0)
        return false;

    const char c = static_cast<char>(state.memory.wordAt(output_data));
    ostream << c;
    ostream.flush();

    // Reset ready bit
    state.memory[output_ready + 3] = std::byte{1};
    // Reset data word
    state.memory[output_data + 3] = std::byte{0};

    return true;
}


void Interpreter::interrupt(const uint32_t cause) { except(cause, ""); }


void Interpreter::except(const uint32_t cause, const std::string& excMsg) {
    // Check if exception handler is a valid address
    const uint32_t handlerAddress = memSectionOffset(MemSection::KTEXT);
    const int32_t pc = state.registers[Register::PC] - 4;
    if (!state.memory.isValid(handlerAddress)) {
        const SourceLocator pcSrc = state.getDebugInfo(pc);
        const std::string what = std::format("{}: {} (unhandled)", causeToString(cause), excMsg);
        throw MasmRuntimeError(what, pc, pcSrc.filename, pcSrc.lineno);
    }

    // Transfer control to exception handler
    state.cp0[Coproc0Register::EPC] = pc; // Save PC before exception
    state.cp0[Coproc0Register::CAUSE] = static_cast<int32_t>(cause);
    state.registers[Register::PC] = static_cast<int32_t>(handlerAddress);
}


void Interpreter::step() {
    uint32_t cause = 0;
    int32_t& pc = state.registers[Register::PC];
    // Update MMIO registers if in MMIO mode and the PC is not in the KTEXT section
    if (ioMode == IOMode::MMIO && static_cast<uint32_t>(pc) < memSectionOffset(MemSection::KTEXT)) {
        // Bit 0 is the interrupt enable bit
        const uint32_t interpEnabled = state.cp0[Coproc0Register::STATUS] & 0x1;
        const uint32_t keyboardEnabled = state.cp0[Coproc0Register::STATUS] &
                                         static_cast<uint32_t>(INTERP_CODE::KEYBOARD_INTERP);
        const uint32_t displayEnabled = state.cp0[Coproc0Register::STATUS] &
                                        static_cast<uint32_t>(INTERP_CODE::DISPLAY_INTERP);
        if (readMMIO() && interpEnabled && keyboardEnabled)
            cause |= static_cast<uint32_t>(INTERP_CODE::KEYBOARD_INTERP);
        if (writeMMIO() && interpEnabled && displayEnabled)
            cause |= static_cast<uint32_t>(INTERP_CODE::DISPLAY_INTERP);
    }

    if (!state.memory.isValid(pc))
        throw ExecExit("Execution terminated (fell off end of program)", -1);
    const SourceLocator pcSrc = state.getDebugInfo(pc);
    if (pc >= TEXT_SEC_END)
        throw MasmRuntimeError("Out of bounds read access", pc, pcSrc.filename, pcSrc.lineno);
    const int32_t instruction = state.memory.wordAt(pc);
    // Increment program counter
    pc += 4;

    if (cause) {
        interrupt(cause);
        return;
    }

    try {
        execInstruction(instruction);
    } catch (ExecExit&) {
        throw;
    } catch (ExecExcept& e) {
        cause = static_cast<uint32_t>(e.cause());
        except(cause, e.what());
    } catch (std::runtime_error& e) {
        throw MasmRuntimeError(e.what(), pc - 4, pcSrc.filename, pcSrc.lineno);
    }
}


void Interpreter::execInstruction(const int32_t instruction) {
    if (instruction == 0x0000000C) {
        // Syscall instruction
        execSyscall(ioMode, state, istream, ostream);
        return;
    }
    if (instruction == 0x42000018) {
        // Eret instruction
        execEret(state.cp0, state.registers);
        return;
    }

    const uint32_t opCode = instruction >> 26 & 0x3F;
    if (opCode == 0x10) {
        // Co-Processor 0 instruction
        const uint32_t rs = instruction >> 21 & 0x1F;
        const uint32_t rt = instruction >> 16 & 0x1F;
        const uint32_t rd = instruction >> 11 & 0x1F;

        // Execute Co-Processor 0 instruction
        execCP0Type(state.cp0, state.registers, rs, rt, rd);
    } else if (opCode == 0x11) {
        // Used to distinguish Co-Processor 1 instruction types
        const uint32_t nextFive = instruction >> 21 & 0x1F;
        // Co-Processor 1 instruction
        if (nextFive == 0x08) {
            const uint32_t tf = instruction >> 16 & 0x01;
            const uint32_t offset = instruction & 0xFFFF;
            // Execute Co-Processor 1 cond immediate instruction
            execCP1CondImmType(state.cp1, state.registers, tf, offset);
        } else if (nextFive == 0x00 || nextFive == 0x04) {
            const uint32_t sub = instruction >> 21 & 0x1F;
            const uint32_t rt = instruction >> 16 & 0x1F;
            const uint32_t fs = instruction >> 11 & 0x1F;
            // Execute Co-Processor 1 reg immediate instruction
            execCP1RegImmType(state.cp1, state.registers, sub, rt, fs);
        } else {
            const uint32_t fmt = instruction >> 21 & 0x1F;
            const uint32_t ft = instruction >> 16 & 0x1F;
            const uint32_t fs = instruction >> 11 & 0x1F;
            const uint32_t fd = instruction >> 6 & 0x1F;
            const uint32_t func = instruction & 0x3F;

            if ((func >> 4 & 0x03) == 0x03)
                // Co-Processor 1 cond type instruction
                execCP1CondType(state.cp1, fmt, ft, fs, func & 0x0F);
            else
                // Co-Processor 1 reg type instruction
                execCP1RegType(state.cp1, fmt, ft, fs, fd, func);
        }
    } else if (opCode == 0x35 || opCode == 0x31 || opCode == 0x3D || opCode == 0x39) {
        // Co-Processor 1 immediate instructions
        const uint32_t base = instruction >> 21 & 0x1F;
        const uint32_t ft = instruction >> 16 & 0x1F;
        const uint32_t offset = instruction & 0xFFFF;

        // Execute Co-Processor 1 immediate instruction
        execCP1ImmType(state.cp1, state.memory, opCode, base, ft, offset);
    } else if (opCode == 0x00) {
        // R-Type instruction
        const uint32_t funct = instruction & 0x3F;
        const uint32_t rs = instruction >> 21 & 0x1F;
        const uint32_t rt = instruction >> 16 & 0x1F;
        const uint32_t rd = instruction >> 11 & 0x1F;
        const uint32_t shamt = instruction >> 6 & 0x1F;

        // Execute R-Type instruction
        execRType(state.registers, funct, rs, rt, rd, shamt);
    } else if (opCode == 0x02 || opCode == 0x03) {
        // J-Type instruction
        execJType(state.registers, opCode, instruction & 0x3FFFFFF);
    } else {
        // I-Type instruction
        const uint32_t rs = instruction >> 21 & 0x1F;
        const uint32_t rt = instruction >> 16 & 0x1F;
        const int32_t immediate = instruction & 0xFFFF;

        // Execute I-Type instruction
        execIType(state.registers, state.memory, opCode, rs, rt, immediate);
    }
}
