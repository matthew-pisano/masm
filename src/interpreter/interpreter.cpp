//
// Created by matthew on 4/24/25.
//

#include "interpreter/interpreter.h"

#include <cstring>
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

    return true;
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


void Interpreter::except(const uint32_t cause) {
    // Check if exception handler is a valid address
    const uint32_t handlerAddress = memSectionOffset(MemSection::KTEXT);
    if (!state.memory.isValid(handlerAddress)) {
        const int32_t pc = state.registers[Register::PC];
        const SourceLocator pcSrc = state.getDebugInfo(pc);
        const std::string what = std::format(
                "Failed to handle exception ({}); no exception handler found at address {}",
                causeToString(cause), hex_to_string(handlerAddress));
        throw MasmRuntimeError(what, pc, pcSrc.filename, pcSrc.lineno);
    }
}


void Interpreter::step() {
    uint32_t cause = 0;
    // Update MMIO registers
    if (ioMode == IOMode::MMIO) {
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

    if (cause)
        except(cause);

    int32_t& pc = state.registers[Register::PC];
    if (!state.memory.isValid(pc))
        throw ExecExit("Execution terminated (fell off end of program)", -1);
    const SourceLocator pcSrc = state.getDebugInfo(pc);
    if (pc >= TEXT_SEC_END)
        throw MasmRuntimeError("Out of bounds read access", pc, pcSrc.filename, pcSrc.lineno);
    const int32_t instruction = state.memory.wordAt(pc);
    // Increment program counter
    pc += 4;

    try {
        if (instruction == 0x0000000C) {
            // Syscall instruction
            execSyscall(ioMode, state, istream, ostream);
            return;
        }
        if (instruction == 0x42000018) {
            // Eret instruction
            execEret();
            return;
        }

        const uint32_t opCode = instruction >> 26 & 0x3F;
        if (opCode == 0x10) {
            // Co-Processor 0 instruction
            const uint32_t rs = instruction >> 21 & 0x1F;
            const uint32_t rt = instruction >> 16 & 0x1F;
            const uint32_t rd = instruction >> 11 & 0x1F;

            // Execute Co-Processor 0 instruction
            execCP0Type(rs, rt, rd);
        } else if (opCode == 0) {
            // R-Type instruction
            const uint32_t funct = instruction & 0x3F;
            const uint32_t rs = instruction >> 21 & 0x1F;
            const uint32_t rt = instruction >> 16 & 0x1F;
            const uint32_t rd = instruction >> 11 & 0x1F;
            const uint32_t shamt = instruction >> 6 & 0x1F;

            // Execute R-Type instruction
            execRType(funct, rs, rt, rd, shamt);
        } else if (opCode == 2 || opCode == 3) {
            // J-Type instruction
            execJType(opCode, instruction & 0x3FFFFFF);
        } else {
            // I-Type instruction
            const uint32_t rs = instruction >> 21 & 0x1F;
            const uint32_t rt = instruction >> 16 & 0x1F;
            const int32_t immediate = instruction & 0xFFFF;

            // Execute I-Type instruction
            execIType(opCode, rs, rt, immediate);
        }
    } catch (ExecExit&) {
        throw;
    } catch (std::runtime_error& e) {
        throw MasmRuntimeError(e.what(), pc - 4, pcSrc.filename, pcSrc.lineno);
    }
}


void Interpreter::execRType(const uint32_t funct, const uint32_t rs, const uint32_t rt,
                            const uint32_t rd, const uint32_t shamt) {
    switch (static_cast<InstructionCode>(funct)) {
        case InstructionCode::ADD: {
            const int64_t extResult = static_cast<int64_t>(state.registers[rs]) +
                                      static_cast<int64_t>(state.registers[rt]);
            if (extResult > INT32_MAX || extResult < INT32_MIN)
                throw std::runtime_error("Integer overflow in ADD instruction");
            state.registers[rd] = state.registers[rs] + state.registers[rt];
            break;
        }
        case InstructionCode::ADDU:
            state.registers[rd] = state.registers[rs] + state.registers[rt];
            break;
        case InstructionCode::AND:
            state.registers[rd] = state.registers[rs] & state.registers[rt];
            break;
        case InstructionCode::DIV: {
            if (state.registers[rt] == 0)
                throw std::runtime_error("Division by zero in DIV instruction");
            state.registers[Register::LO] = state.registers[rs] / state.registers[rt];
            state.registers[Register::HI] = state.registers[rs] % state.registers[rt];
            break;
        }
        case InstructionCode::DIVU: {
            if (state.registers[rt] == 0)
                throw std::runtime_error("Division by zero in DIVU instruction");
            const uint32_t rsVal = state.registers[rs];
            const uint32_t rtVal = state.registers[rt];
            state.registers[Register::LO] = static_cast<int32_t>(rsVal / rtVal);
            state.registers[Register::HI] = static_cast<int32_t>(rsVal % rtVal);
            break;
        }
        case InstructionCode::MFHI: {
            state.registers[rd] = state.registers[Register::HI];
            break;
        }
        case InstructionCode::MFLO: {
            state.registers[rd] = state.registers[Register::LO];
            break;
        }
        case InstructionCode::MTHI: {
            state.registers[Register::HI] = state.registers[rs];
            break;
        }
        case InstructionCode::MTLO: {
            state.registers[Register::LO] = state.registers[rs];
            break;
        }
        case InstructionCode::MULT: {
            const int64_t rsVal = state.registers[rs];
            const int64_t rtVal = state.registers[rt];
            const int64_t result = rsVal * rtVal;
            state.registers[Register::LO] = static_cast<int32_t>(result & 0xFFFFFFFF);
            state.registers[Register::HI] = static_cast<int32_t>(result >> 32 & 0xFFFFFFFF);
            break;
        }
        case InstructionCode::MULTU: {
            const uint64_t rsVal = static_cast<uint32_t>(state.registers[rs]);
            const uint64_t rtVal = static_cast<uint32_t>(state.registers[rt]);
            const uint64_t result = rsVal * rtVal;
            state.registers[Register::LO] = static_cast<int32_t>(result & 0xFFFFFFFF);
            state.registers[Register::HI] = static_cast<int32_t>(result >> 32 & 0xFFFFFFFF);
            break;
        }
        case InstructionCode::NOR:
            state.registers[rd] = ~(state.registers[rs] | state.registers[rt]);
            break;
        case InstructionCode::OR:
            state.registers[rd] = state.registers[rs] | state.registers[rt];
            break;
        case InstructionCode::SLL:
            state.registers[rd] = state.registers[rt] << shamt;
            break;
        case InstructionCode::SLLV:
            // Shift up to 5 bits (32 places)
            state.registers[rd] = state.registers[rt] << (state.registers[rs] & 0x1F);
            break;
        case InstructionCode::SRA: {
            // Arithmetic right shift (sign-extended)
            const bool sign = state.registers[rt] & 0x80000000;
            const uint32_t signExtend = sign ? 0xFFFFFFFF << (32 - shamt) : 0;
            state.registers[rd] = static_cast<int32_t>(state.registers[rt] >> shamt | signExtend);
            break;
        }
        case InstructionCode::SRAV: {
            // Arithmetic right shift (sign-extended)
            const bool sign = state.registers[rt] & 0x80000000;
            const uint32_t vShamt = state.registers[rs] & 0x1F;
            const uint32_t signExtend = sign ? 0xFFFFFFFF << (32 - vShamt) : 0;
            state.registers[rd] = static_cast<int32_t>(state.registers[rt] >> vShamt | signExtend);
            break;
        }
        case InstructionCode::SRL:
            // Logical right shift (zero-extended)
            state.registers[rd] = state.registers[rt] >> shamt;
            break;
        case InstructionCode::SRLV:
            // Logical right shift (zero-extended)
            state.registers[rd] = state.registers[rt] >> (state.registers[rs] & 0x1F);
            break;
        case InstructionCode::SUB: {
            const int64_t extResult = static_cast<int64_t>(state.registers[rs]) -
                                      static_cast<int64_t>(state.registers[rt]);
            if (extResult > INT32_MAX || extResult < INT32_MIN)
                throw std::runtime_error("Integer overflow in SUB instruction");
            state.registers[rd] = state.registers[rs] - state.registers[rt];
            break;
        }
        case InstructionCode::SUBU:
            state.registers[rd] = state.registers[rs] - state.registers[rt];
            break;
        case InstructionCode::XOR:
            state.registers[rd] = state.registers[rs] ^ state.registers[rt];
            break;
        case InstructionCode::SLT:
            state.registers[rd] = state.registers[rs] < state.registers[rt] ? 1 : 0;
            break;
        case InstructionCode::SLTU: {
            const uint32_t rsVal = state.registers[rs];
            const uint32_t rtVal = state.registers[rt];
            state.registers[rd] = rsVal < rtVal ? 1 : 0;
            break;
        }
        case InstructionCode::JR:
            // Jump to the address in rs
            state.registers[Register::PC] = state.registers[rs];
            break;
        case InstructionCode::JALR: {
            // Link current PC to RA register
            state.registers[Register::RA] = state.registers[Register::PC]; // Already incremented
            // Save stack pointer to frame pointer
            state.registers[Register::FP] = state.registers[Register::SP];
            // Jump to the address in rs
            state.registers[Register::PC] = state.registers[rs];
            break;
        }
        default:
            // Should never be reached
            throw std::runtime_error("Unknown R-Type instruction " + std::to_string(funct));
    }
}


void Interpreter::execIType(const uint32_t opCode, const uint32_t rs, const uint32_t rt,
                            const int32_t immediate) {
    int32_t signExtImm = immediate;
    // Sign-extend immediate value
    if (signExtImm & 0x8000)
        signExtImm |= static_cast<int32_t>(0xFFFF0000);

    switch (static_cast<InstructionCode>(opCode)) {
        case InstructionCode::ADDI: {
            const int64_t extResult = static_cast<int64_t>(state.registers[rs]) + signExtImm;
            if (extResult > INT32_MAX || extResult < INT32_MIN)
                throw std::runtime_error("Integer overflow in ADDI instruction");
            state.registers[rt] = state.registers[rs] + signExtImm;
            break;
        }
        case InstructionCode::ADDIU:
            state.registers[rt] = state.registers[rs] + signExtImm;
            break;
        case InstructionCode::ANDI:
            state.registers[rt] = state.registers[rs] & immediate;
            break;
        case InstructionCode::ORI:
            state.registers[rt] = state.registers[rs] | immediate;
            break;
        case InstructionCode::XORI:
            state.registers[rt] = state.registers[rs] ^ immediate;
            break;
        case InstructionCode::SLTI:
            state.registers[rt] = state.registers[rs] < signExtImm ? 1 : 0;
            break;
        case InstructionCode::SLTIU: {
            const uint32_t rsVal = state.registers[rs];
            const uint32_t uSignExtImm = signExtImm;
            state.registers[rt] = rsVal < uSignExtImm ? 1 : 0;
            break;
        }
        case InstructionCode::LB: {
            // Convert to signed for sign extension
            const int8_t result = state.memory.byteAt(state.registers[rs] + immediate);
            state.registers[rt] = result;
            break;
        }
        case InstructionCode::LH: {
            // Convert to signed for sign extension
            const int16_t result = state.memory.halfAt(state.registers[rs] + immediate);
            state.registers[rt] = result;
            break;
        }
        case InstructionCode::LW:
            state.registers[rt] = state.memory.wordAt(state.registers[rs] + immediate);
            break;
        case InstructionCode::LBU:
            state.registers[rt] = state.memory.byteAt(state.registers[rs] + immediate);
            break;
        case InstructionCode::LHU:
            state.registers[rt] = state.memory.halfAt(state.registers[rs] + immediate);
            break;
        case InstructionCode::LUI:
            // Load upper immediate
            state.registers[rt] = signExtImm << 16;
            break;
        case InstructionCode::SB:
            state.memory.byteTo(state.registers[rs] + immediate,
                                static_cast<int8_t>(state.registers[rt]));
            break;
        case InstructionCode::SH:
            state.memory.halfTo(state.registers[rs] + immediate,
                                static_cast<int16_t>(state.registers[rt]));
            break;
        case InstructionCode::SW:
            state.memory.wordTo(state.registers[rs] + immediate, state.registers[rt]);
            break;
        case InstructionCode::BEQ:
            if (state.registers[rs] == state.registers[rt])
                state.registers[Register::PC] = state.registers[Register::PC] + (signExtImm << 2);
            break;
        case InstructionCode::BNE:
            if (state.registers[rs] != state.registers[rt])
                state.registers[Register::PC] = state.registers[Register::PC] + (signExtImm << 2);
            break;
        default:
            // Should never be reached
            throw std::runtime_error("Unknown I-Type instruction " + std::to_string(opCode));
    }
}


void Interpreter::execJType(const uint32_t opCode, const uint32_t address) {
    if (opCode == InstructionCode::JAL) {
        // Link current PC to RA register
        state.registers[Register::RA] = state.registers[Register::PC]; // PC incremented earlier
        // Save stack pointer to frame pointer
        state.registers[Register::FP] = state.registers[Register::SP];
    }

    // Jump to the target address
    state.registers[Register::PC] = (state.registers[Register::PC] & 0xF0000000) | address << 2;
}


void Interpreter::execEret() {
    // Restore the PC from the saved address in the EPC register
    state.registers[Register::PC] = state.cp0[Coproc0Register::EPC];
    state.cp0[Coproc0Register::EPC] = 0;

    // Clear the cause register
    state.cp0[Coproc0Register::CAUSE] = 0;
}


void Interpreter::execCP0Type(const uint32_t rs, const uint32_t rt, const uint32_t rd) {
    switch (static_cast<InstructionCode>(rs)) {
        case InstructionCode::MFC0: {
            // Move from CP0
            state.registers[rt] = state.cp0[rd];
            break;
        }
        case InstructionCode::MTC0: {
            // Move to CP0
            state.cp0[rd] = state.registers[rt];
            break;
        }
        default:
            throw std::runtime_error("Unknown Co-Processor 0 instruction " + std::to_string(rs));
    }
}
