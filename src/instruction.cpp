//
// Created by matthew on 4/15/25.
//

#include "instruction.h"

#include <map>
#include <stdexcept>

#include "utils.h"


std::map<std::string, InstructionOp> instructionNameMap = {
        // Arithmetic and Logical Instructions
        {"add", {InstructionType::R_TYPE, 0x20, 4}},
        {"addu", {InstructionType::R_TYPE, 0x21, 4}},
        {"addi", {InstructionType::I_TYPE, 0x08, 4}},
        {"addiu", {InstructionType::I_TYPE, 0x09, 4}},
        {"and", {InstructionType::R_TYPE, 0x24, 4}},
        {"andi", {InstructionType::I_TYPE, 0x0C, 4}},
        {"div", {InstructionType::R_TYPE, 0x1A, 4}},
        {"divu", {InstructionType::R_TYPE, 0x1B, 4}},
        {"mult", {InstructionType::R_TYPE, 0x18, 4}},
        {"multu", {InstructionType::R_TYPE, 0x19, 4}},
        {"nor", {InstructionType::R_TYPE, 0x27, 4}},
        {"or", {InstructionType::R_TYPE, 0x25, 4}},
        {"ori", {InstructionType::I_TYPE, 0x0D, 4}},
        {"sll", {InstructionType::R_TYPE, 0x00, 4}},
        {"sllv", {InstructionType::R_TYPE, 0x04, 4}},
        {"sra", {InstructionType::R_TYPE, 0x03, 4}},
        {"srav", {InstructionType::R_TYPE, 0x07, 4}},
        {"srl", {InstructionType::R_TYPE, 0x02, 4}},
        {"srlv", {InstructionType::R_TYPE, 0x06, 4}},
        {"sub", {InstructionType::R_TYPE, 0x22, 4}},
        {"subu", {InstructionType::R_TYPE, 0x23, 4}},
        {"xor", {InstructionType::R_TYPE, 0x26, 4}},
        {"xori", {InstructionType::I_TYPE, 0x0E, 4}},

        // Comparison Instructions
        {"slt", {InstructionType::R_TYPE, 0x2a, 4}},
        {"sltu", {InstructionType::R_TYPE, 0x29, 4}},
        {"slti", {InstructionType::I_TYPE, 0x0a, 4}},
        {"sltiu", {InstructionType::I_TYPE, 0x09, 4}},

        // Branch Instructions
        {"beq", {InstructionType::SWAPPED_I_TYPE, 0x04, 4}},
        {"bgtz", {InstructionType::SHORT_I_TYPE, 0x07, 4}},
        {"blez", {InstructionType::SHORT_I_TYPE, 0x06, 4}},
        {"bltz", {InstructionType::SHORT_I_TYPE, 0x07, 4}},
        {"bgez", {InstructionType::SHORT_I_TYPE, 0x01, 4}},
        {"bne", {InstructionType::SWAPPED_I_TYPE, 0x05, 4}},

        // Jump Instructions
        {"j", {InstructionType::J_TYPE, 0x02, 4}},
        {"jal", {InstructionType::J_TYPE, 0x03, 4}},
        {"jalr", {InstructionType::R_TYPE, 0x09, 4}},
        {"jr", {InstructionType::R_TYPE, 0x08, 4}},

        // Load Instructions
        {"lb", {InstructionType::I_TYPE, 0x20, 4}},
        {"lbu", {InstructionType::I_TYPE, 0x24, 4}},
        {"lh", {InstructionType::I_TYPE, 0x21, 4}},
        {"lhu", {InstructionType::I_TYPE, 0x25, 4}},
        {"lw", {InstructionType::I_TYPE, 0x23, 4}},
        {"lui", {InstructionType::SHORT_I_TYPE, 0x0f, 4}},

        // Store Instructions
        {"sb", {InstructionType::I_TYPE, 0x28, 4}},
        {"sh", {InstructionType::I_TYPE, 0x29, 4}},
        {"sw", {InstructionType::I_TYPE, 0x2B, 4}},

        // Syscall
        {"syscall", {InstructionType::SYSCALL, 0, 4}},

        // Pseudo Instructions
        {"li", {InstructionType::PSEUDO, 0, 4}},
        {"la", {InstructionType::PSEUDO, 0, 8}},
        {"blt", {InstructionType::PSEUDO, 0, 8}},
        {"bgt", {InstructionType::PSEUDO, 0, 8}},
        {"bge", {InstructionType::PSEUDO, 0, 8}},
        {"ble", {InstructionType::PSEUDO, 0, 8}},

};


InstructionOp nameToInstructionOp(const std::string& name) {
    if (instructionNameMap.contains(name))
        return instructionNameMap[name];
    throw std::runtime_error("Unknown instruction " + name);
}


void validateInstruction(const Token& instruction, const std::vector<Token>& args) {
    InstructionType instructionType = nameToInstructionOp(instruction.value).type;

    switch (instructionType) {
        case InstructionType::R_TYPE:
            if (!tokenTypeMatch({TokenType::REGISTER, TokenType::REGISTER, TokenType::REGISTER},
                                args))
                throw std::runtime_error("Invalid format for R-Type instruction " +
                                         instruction.value);
            break;
        case InstructionType::SWAPPED_I_TYPE:
        case InstructionType::I_TYPE:
            if (!tokenTypeMatch({TokenType::REGISTER, TokenType::REGISTER, TokenType::IMMEDIATE},
                                args))
                throw std::runtime_error("Invalid format for I-Type instruction " +
                                         instruction.value);
            break;
        case InstructionType::J_TYPE:
            if (!tokenTypeMatch({TokenType::LABELREF}, args))
                throw std::runtime_error("Invalid format for J-Type instruction " +
                                         instruction.value);
            break;
        case InstructionType::SHORT_I_TYPE:
            if (!tokenTypeMatch({TokenType::REGISTER, TokenType::IMMEDIATE}, args))
                throw std::runtime_error("Invalid format for I-Type instruction " +
                                         instruction.value);
            break;
        case InstructionType::SYSCALL:
            if (!tokenTypeMatch({}, args))
                throw std::runtime_error("Invalid format for Syscall");
            break;
        case InstructionType::PSEUDO:
            validatePseudoInstruction(instruction, args);
            break;
    }
}


void validatePseudoInstruction(const Token& instruction, const std::vector<Token>& args) {
    std::vector<std::string> branchPseudoInstrs = {"blt", "bgt", "ble", "bge"};
    std::string instructionName = instruction.value;

    if (instructionName == "li" &&
        !tokenTypeMatch({TokenType::REGISTER, TokenType::IMMEDIATE}, args))
        throw std::runtime_error("Invalid format for instruction " + instruction.value);
    if (instructionName == "la" &&
        !tokenTypeMatch({TokenType::REGISTER, TokenType::LABELREF}, args))
        throw std::runtime_error("Invalid format for instruction " + instruction.value);
    if (std::ranges::find(branchPseudoInstrs, instructionName) != branchPseudoInstrs.end() &&
        !tokenTypeMatch({TokenType::REGISTER, TokenType::REGISTER, TokenType::LABELREF}, args))
        throw std::runtime_error("Invalid format for instruction " + instruction.value);
}
