//
// Created by matthew on 4/24/25.
//

#include "interpreter.h"

#include <stdexcept>

#include "instruction.h"


int Interpreter::interpret(const MemLayout& layout) {
    state.memory.loadProgram(layout);
    // Initialize PC to the start of the text section
    state.registers[Register::PC] = memSectionOffset(MemSection::TEXT);
    state.registers[Register::SP] = 0x7FFFFFFC;
    state.registers[Register::GP] = 0x10008000;

    return 0;
}


void Interpreter::step() {
    const int32_t instruction = state.memory.wordAt(state.registers[Register::PC]);
    state.registers[Register::PC] += 4;

    if (instruction == 0x0000000C) {
        // Syscall instruction
        syscall();
        return;
    }

    const uint32_t opCode = (instruction >> 26) & 0x3F;
    if (opCode == 0) {
        // R-Type instruction
        const uint32_t funct = instruction & 0x3F;
        const uint32_t rs = (instruction >> 21) & 0x1F;
        const uint32_t rt = (instruction >> 16) & 0x1F;
        const uint32_t rd = (instruction >> 11) & 0x1F;
        const uint32_t shamt = (instruction >> 6) & 0x1F;

        // Execute R-Type instruction
        execRType(funct, rs, rt, rd, shamt);
    } else if (opCode == 2 || opCode == 3) {
        // J-Type instruction
        execJType(opCode, instruction & 0x3FFFFFF);
    } else {
        // I-Type instruction
        const uint32_t rs = (instruction >> 21) & 0x1F;
        const uint32_t rt = (instruction >> 16) & 0x1F;
        const int32_t immediate = instruction & 0xFFFF;

        // Execute I-Type instruction
        execIType(opCode, rs, rt, immediate);
    }
}


void Interpreter::execRType(uint32_t funct, uint32_t rs, uint32_t rt, uint32_t rd, uint32_t shamt) {
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
    state.registers[Register::PC] = (state.registers[Register::PC] & 0xF0000000) | (address << 2);
}


void Interpreter::syscall() {}
