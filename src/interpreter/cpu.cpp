//
// Created by matthew on 6/7/25.
//

#include "interpreter/cpu.h"

#include <stdexcept>

#include "exceptions.h"
#include "parser/instruction.h"
#include "utils.h"


std::map<std::string, Register> RegisterFile::nameToIndex = {
        {"zero", Register::ZERO}, {"at", Register::AT}, {"v0", Register::V0}, {"v1", Register::V1},
        {"a0", Register::A0},     {"a1", Register::A1}, {"a2", Register::A2}, {"a3", Register::A3},
        {"t0", Register::T0},     {"t1", Register::T1}, {"t2", Register::T2}, {"t3", Register::T3},
        {"t4", Register::T4},     {"t5", Register::T5}, {"t6", Register::T6}, {"t7", Register::T7},
        {"s0", Register::S0},     {"s1", Register::S1}, {"s2", Register::S2}, {"s3", Register::S3},
        {"s4", Register::S4},     {"s5", Register::S5}, {"s6", Register::S6}, {"s7", Register::S7},
        {"t8", Register::T8},     {"t9", Register::T9}, {"k0", Register::K0}, {"k1", Register::K1},
        {"gp", Register::GP},     {"sp", Register::SP}, {"fp", Register::FP}, {"ra", Register::RA}};


int RegisterFile::indexFromName(const std::string& name) {
    if (name.starts_with("f") && isSignedInteger(name.substr(1)) &&
        std::stoi(name.substr(1)) >= 0) {
        // Handle floating point registers ($f0-$f31)
        return std::stoi(name.substr(1));
    }
    if (!nameToIndex.contains(name))
        throw std::runtime_error("Unknown register " + name);

    return static_cast<int>(nameToIndex[name]);
}


// Core register access
int32_t RegisterFile::operator[](const uint32_t index) const { return registers.at(index); }
int32_t& RegisterFile::operator[](const uint32_t index) { return registers.at(index); }

int32_t RegisterFile::operator[](const Register index) const {
    return registers.at(static_cast<uint32_t>(index));
}
int32_t& RegisterFile::operator[](const Register index) {
    return registers.at(static_cast<uint32_t>(index));
}


void execRType(RegisterFile& registers, const uint32_t funct, const uint32_t rs, const uint32_t rt,
               const uint32_t rd, const uint32_t shamt) {
    switch (static_cast<InstructionCode>(funct)) {
        case InstructionCode::ADD: {
            const int64_t extResult =
                    static_cast<int64_t>(registers[rs]) + static_cast<int64_t>(registers[rt]);
            if (extResult > INT32_MAX || extResult < INT32_MIN)
                throw ExecExcept("Integer overflow in ADD instruction",
                                 EXCEPT_CODE::ARITHMETIC_OVERFLOW_EXCEPTION);
            registers[rd] = registers[rs] + registers[rt];
            break;
        }
        case InstructionCode::ADDU:
            registers[rd] = registers[rs] + registers[rt];
            break;
        case InstructionCode::AND:
            registers[rd] = registers[rs] & registers[rt];
            break;
        case InstructionCode::DIV: {
            if (registers[rt] == 0)
                throw ExecExcept("Division by zero in DIV instruction",
                                 EXCEPT_CODE::DIVIDE_BY_ZERO_EXCEPTION);
            registers[Register::LO] = registers[rs] / registers[rt];
            registers[Register::HI] = registers[rs] % registers[rt];
            break;
        }
        case InstructionCode::DIVU: {
            if (registers[rt] == 0)
                throw ExecExcept("Division by zero in DIVU instruction",
                                 EXCEPT_CODE::DIVIDE_BY_ZERO_EXCEPTION);
            const uint32_t rsVal = registers[rs];
            const uint32_t rtVal = registers[rt];
            registers[Register::LO] = static_cast<int32_t>(rsVal / rtVal);
            registers[Register::HI] = static_cast<int32_t>(rsVal % rtVal);
            break;
        }
        case InstructionCode::MFHI: {
            registers[rd] = registers[Register::HI];
            break;
        }
        case InstructionCode::MFLO: {
            registers[rd] = registers[Register::LO];
            break;
        }
        case InstructionCode::MTHI: {
            registers[Register::HI] = registers[rs];
            break;
        }
        case InstructionCode::MTLO: {
            registers[Register::LO] = registers[rs];
            break;
        }
        case InstructionCode::MULT: {
            const int64_t rsVal = registers[rs];
            const int64_t rtVal = registers[rt];
            const int64_t result = rsVal * rtVal;
            registers[Register::LO] = static_cast<int32_t>(result & 0xFFFFFFFF);
            registers[Register::HI] = static_cast<int32_t>(result >> 32 & 0xFFFFFFFF);
            break;
        }
        case InstructionCode::MULTU: {
            const uint64_t rsVal = static_cast<uint32_t>(registers[rs]);
            const uint64_t rtVal = static_cast<uint32_t>(registers[rt]);
            const uint64_t result = rsVal * rtVal;
            registers[Register::LO] = static_cast<int32_t>(result & 0xFFFFFFFF);
            registers[Register::HI] = static_cast<int32_t>(result >> 32 & 0xFFFFFFFF);
            break;
        }
        case InstructionCode::NOR:
            registers[rd] = ~(registers[rs] | registers[rt]);
            break;
        case InstructionCode::OR:
            registers[rd] = registers[rs] | registers[rt];
            break;
        case InstructionCode::SLL:
            registers[rd] = registers[rt] << shamt;
            break;
        case InstructionCode::SLLV:
            // Shift up to 5 bits (32 places)
            registers[rd] = registers[rt] << (registers[rs] & 0x1F);
            break;
        case InstructionCode::SRA: {
            // Arithmetic right shift (sign-extended)
            const bool sign = registers[rt] & 0x80000000;
            const uint32_t signExtend = sign ? 0xFFFFFFFF << (32 - shamt) : 0;
            registers[rd] = static_cast<int32_t>(registers[rt] >> shamt | signExtend);
            break;
        }
        case InstructionCode::SRAV: {
            // Arithmetic right shift (sign-extended)
            const bool sign = registers[rt] & 0x80000000;
            const uint32_t vShamt = registers[rs] & 0x1F;
            const uint32_t signExtend = sign ? 0xFFFFFFFF << (32 - vShamt) : 0;
            registers[rd] = static_cast<int32_t>(registers[rt] >> vShamt | signExtend);
            break;
        }
        case InstructionCode::SRL:
            // Logical right shift (zero-extended)
            registers[rd] = registers[rt] >> shamt;
            break;
        case InstructionCode::SRLV:
            // Logical right shift (zero-extended)
            registers[rd] = registers[rt] >> (registers[rs] & 0x1F);
            break;
        case InstructionCode::SUB: {
            const int64_t extResult =
                    static_cast<int64_t>(registers[rs]) - static_cast<int64_t>(registers[rt]);
            if (extResult > INT32_MAX || extResult < INT32_MIN)
                throw ExecExcept("Integer overflow in SUB instruction",
                                 EXCEPT_CODE::ARITHMETIC_OVERFLOW_EXCEPTION);
            registers[rd] = registers[rs] - registers[rt];
            break;
        }
        case InstructionCode::SUBU:
            registers[rd] = registers[rs] - registers[rt];
            break;
        case InstructionCode::XOR:
            registers[rd] = registers[rs] ^ registers[rt];
            break;
        case InstructionCode::SLT:
            registers[rd] = registers[rs] < registers[rt] ? 1 : 0;
            break;
        case InstructionCode::SLTU: {
            const uint32_t rsVal = registers[rs];
            const uint32_t rtVal = registers[rt];
            registers[rd] = rsVal < rtVal ? 1 : 0;
            break;
        }
        case InstructionCode::JR:
            // Jump to the address in rs
            registers[Register::PC] = registers[rs];
            break;
        case InstructionCode::JALR: {
            // Link current PC to RA register
            registers[Register::RA] = registers[Register::PC]; // Already incremented
            // Jump to the address in rs
            registers[Register::PC] = registers[rs];
            break;
        }
        default:
            // Should never be reached
            throw std::runtime_error("Unknown R-Type instruction " + std::to_string(funct));
    }
}


void execIType(RegisterFile& registers, Memory& memory, const uint32_t opCode, const uint32_t rs,
               const uint32_t rt, const int32_t immediate) {
    int32_t signExtImm = immediate;
    // Sign-extend immediate value
    if (signExtImm & 0x8000)
        signExtImm |= static_cast<int32_t>(0xFFFF0000);

    switch (static_cast<InstructionCode>(opCode)) {
        case InstructionCode::ADDI: {
            const int64_t extResult = static_cast<int64_t>(registers[rs]) + signExtImm;
            if (extResult > INT32_MAX || extResult < INT32_MIN)
                throw ExecExcept("Integer overflow in ADDI instruction",
                                 EXCEPT_CODE::ARITHMETIC_OVERFLOW_EXCEPTION);
            registers[rt] = registers[rs] + signExtImm;
            break;
        }
        case InstructionCode::ADDIU:
            registers[rt] = registers[rs] + signExtImm;
            break;
        case InstructionCode::ANDI:
            registers[rt] = registers[rs] & immediate;
            break;
        case InstructionCode::ORI:
            registers[rt] = registers[rs] | immediate;
            break;
        case InstructionCode::XORI:
            registers[rt] = registers[rs] ^ immediate;
            break;
        case InstructionCode::SLTI:
            registers[rt] = registers[rs] < signExtImm ? 1 : 0;
            break;
        case InstructionCode::SLTIU: {
            const uint32_t rsVal = registers[rs];
            const uint32_t uSignExtImm = signExtImm;
            registers[rt] = rsVal < uSignExtImm ? 1 : 0;
            break;
        }
        case InstructionCode::LB: {
            // Convert to signed for sign extension
            const int8_t result = memory.byteAt(registers[rs] + immediate);
            registers[rt] = result;
            break;
        }
        case InstructionCode::LH: {
            // Convert to signed for sign extension
            const int16_t result = memory.halfAt(registers[rs] + immediate);
            registers[rt] = result;
            break;
        }
        case InstructionCode::LW:
            registers[rt] = memory.wordAt(registers[rs] + immediate);
            break;
        case InstructionCode::LBU:
            registers[rt] = memory.byteAt(registers[rs] + immediate);
            break;
        case InstructionCode::LHU:
            registers[rt] = memory.halfAt(registers[rs] + immediate);
            break;
        case InstructionCode::LUI:
            // Load upper immediate
            registers[rt] = signExtImm << 16;
            break;
        case InstructionCode::SB:
            memory.byteTo(registers[rs] + immediate, static_cast<int8_t>(registers[rt]));
            break;
        case InstructionCode::SH:
            memory.halfTo(registers[rs] + immediate, static_cast<int16_t>(registers[rt]));
            break;
        case InstructionCode::SW:
            memory.wordTo(registers[rs] + immediate, registers[rt]);
            break;
        case InstructionCode::BEQ:
            if (registers[rs] == registers[rt])
                registers[Register::PC] += signExtImm << 2; // Offset is word-aligned, so shift by 2
            break;
        case InstructionCode::BNE:
            if (registers[rs] != registers[rt])
                registers[Register::PC] += signExtImm << 2; // Offset is word-aligned, so shift by 2
            break;
        default:
            // Should never be reached
            throw std::runtime_error("Unknown I-Type instruction " + std::to_string(opCode));
    }
}


void execJType(RegisterFile& registers, const uint32_t opCode, const uint32_t address) {
    if (opCode == InstructionCode::JAL) {
        // Link current PC to RA register
        registers[Register::RA] = registers[Register::PC]; // PC incremented earlier
    }

    // Jump to the target address
    registers[Register::PC] = (registers[Register::PC] & 0xF0000000) | address << 2;
}
