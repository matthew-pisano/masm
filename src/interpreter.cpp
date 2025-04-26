//
// Created by matthew on 4/24/25.
//

#include "interpreter.h"

#include <stdexcept>

#include "instruction.h"
#include "syscall.h"


int Interpreter::interpret(const MemLayout& layout) {
    state.memory.loadProgram(layout);
    // Initialize PC to the start of the text section
    state.registers[Register::PC] = static_cast<int32_t>(memSectionOffset(MemSection::TEXT));
    state.registers[Register::SP] = 0x7FFFFFFC;
    state.registers[Register::GP] = 0x10008000;

    while (true) {
        try {
            step();
        } catch (ExecExit& e) {
            return e.code();
        }
    }
}


void Interpreter::step() {
    const int32_t instruction = state.memory.wordAt(state.registers[Register::PC]);
    if (instruction == 0)
        throw std::runtime_error("Execution terminated (fell off end of program)");

    state.registers[Register::PC] += 4;

    if (instruction == 0x0000000C) {
        // Syscall instruction
        syscall();
        return;
    }

    const uint32_t opCode = instruction >> 26 & 0x3F;
    if (opCode == 0) {
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
            state.registers[Register::LO] = static_cast<uint32_t>(state.registers[rs]) /
                                            static_cast<uint32_t>(state.registers[rt]);
            state.registers[Register::HI] = static_cast<uint32_t>(state.registers[rs]) %
                                            static_cast<uint32_t>(state.registers[rt]);
            break;
        }
        case InstructionCode::MULT: {
            state.registers[Register::LO] = static_cast<int64_t>(state.registers[rs]) *
                                                    static_cast<int64_t>(state.registers[rt]) &
                                            0xFFFFFFFF;
            state.registers[Register::HI] =
                    static_cast<int64_t>(state.registers[rs]) *
                                    static_cast<int64_t>(state.registers[rt]) >>
                            32 &
                    0xFFFFFFFF;
            break;
        }
        case InstructionCode::MULTU: {
            state.registers[Register::LO] = static_cast<uint64_t>(state.registers[rs]) *
                                                    static_cast<uint64_t>(state.registers[rt]) &
                                            0xFFFFFFFF;
            state.registers[Register::HI] = (static_cast<uint64_t>(state.registers[rs]) *
                                                     static_cast<uint64_t>(state.registers[rt]) >>
                                             32) &
                                            0xFFFFFFFF;
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
            state.registers[rd] =
                    (state.registers[rt] >> shamt) | (sign ? 0xFFFFFFFF << (32 - shamt) : 0);
            break;
        }
        case InstructionCode::SRAV: {
            // Arithmetic right shift (sign-extended)
            const bool signShift = state.registers[rt] & 0x80000000;
            state.registers[rd] =
                    (state.registers[rt] >> (state.registers[rs] & 0x1F)) |
                    (signShift ? 0xFFFFFFFF << (32 - (state.registers[rs] & 0x1F)) : 0);
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
        case InstructionCode::SLTU:
            state.registers[rd] = static_cast<uint32_t>(state.registers[rs]) <
                                                  static_cast<uint32_t>(state.registers[rt])
                                          ? 1
                                          : 0;
            break;
        case InstructionCode::JR:
            // Jump to the address in rs
            state.registers[Register::PC] = state.registers[rs];
            break;
        case InstructionCode::JALR: {
            // Jump and link
            state.registers[Register::RA] = state.registers[Register::PC];
            state.registers[Register::PC] = state.registers[rs];
            break;
        }

        default:
            throw std::runtime_error("Unknown R-Type instruction " + std::to_string(funct));
    }
}


void Interpreter::execIType(const uint32_t opCode, const uint32_t rs, const uint32_t rt,
                            const int32_t immediate) {

    int32_t signExtImmediate = immediate;
    // Sign-extend immediate value
    if (signExtImmediate & 0x8000)
        signExtImmediate |= 0xFFFF0000;

    switch (static_cast<InstructionCode>(opCode)) {
        case InstructionCode::ADDI: {
            const int64_t extResult = static_cast<int64_t>(state.registers[rs]) + signExtImmediate;
            if (extResult > INT32_MAX || extResult < INT32_MIN)
                throw std::runtime_error("Integer overflow in ADDI instruction");
            state.registers[rt] = state.registers[rs] + signExtImmediate;
            break;
        }
        case InstructionCode::ADDIU:
            state.registers[rt] = state.registers[rs] + signExtImmediate;
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
            state.registers[rt] = (state.registers[rs] < signExtImmediate) ? 1 : 0;
            break;
        case InstructionCode::SLTIU:
            state.registers[rt] =
                    (state.registers[rs] < static_cast<uint32_t>(signExtImmediate)) ? 1 : 0;
            break;
        case InstructionCode::LB:
            // Convert to signed for sign extension
            state.registers[rt] =
                    static_cast<int8_t>(state.memory.byteAt(state.registers[rs] + immediate));
            break;
        case InstructionCode::LH:
            // Convert to signed for sign extension
            state.registers[rt] =
                    static_cast<int16_t>(state.memory.halfAt(state.registers[rs] + immediate));
            break;
        case InstructionCode::LW:
            state.registers[rt] = state.memory.wordAt(state.registers[rs] + immediate);
            break;
        case InstructionCode::LBU:
            state.registers[rt] = state.memory.byteAt(state.registers[rs] + immediate);
            break;
        case InstructionCode::LHU:
            state.registers[rt] = state.memory.halfAt(state.registers[rs] + immediate);
            break;
        case InstructionCode::SB:
            state.memory.byteTo(state.registers[rs] + immediate,
                                static_cast<uint8_t>(state.registers[rt]));
            break;
        case InstructionCode::SH:
            state.memory.halfTo(state.registers[rs] + immediate,
                                static_cast<uint16_t>(state.registers[rt]));
            break;
        case InstructionCode::SW:
            state.memory.wordTo(state.registers[rs] + immediate, state.registers[rt]);
            break;
        case InstructionCode::BEQ:
            if (state.registers[rs] == state.registers[rt])
                state.registers[Register::PC] =
                        state.registers[Register::PC] + (signExtImmediate << 2);
            break;
        case InstructionCode::BNE:
            if (state.registers[rs] != state.registers[rt])
                state.registers[Register::PC] =
                        state.registers[Register::PC] + (signExtImmediate << 2);
            break;
        default:
            throw std::runtime_error("Unknown I-Type instruction " + std::to_string(opCode));
    }
}


void Interpreter::execJType(const uint32_t opCode, const uint32_t address) {
    if (opCode == InstructionCode::JAL)
        // Jump and link
        state.registers[Register::RA] = state.registers[Register::PC]; // PC incremented earlier

    // Jump to the target address
    state.registers[Register::PC] = (state.registers[Register::PC] & 0xF0000000) | address << 2;
}


void Interpreter::syscall() {
    int32_t syscallCode = state.registers[Register::V0];
    switch (static_cast<Syscall>(syscallCode)) {
        case Syscall::PRINT_INT:
            printIntSyscall(state);
            break;
        case Syscall::PRINT_STRING:
            printStringSyscall(state);
            break;
        case Syscall::READ_INT:
            readIntSyscall(state);
            break;
        case Syscall::READ_STRING:
            readStringSyscall(state);
            break;
        case Syscall::EXIT:
            exitSyscall(state);
            break;
        case Syscall::PRINT_CHAR:
            printCharSyscall(state);
            break;
        case Syscall::READ_CHAR:
            readCharSyscall(state);
            break;
        case Syscall::EXIT_VAL:
            exitValSyscall(state);
            break;
    }
}
