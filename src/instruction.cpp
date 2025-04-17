//
// Created by matthew on 4/15/25.
//

#include "instruction.h"

#include <map>
#include <stdexcept>


std::map<InstructionType, std::vector<std::vector<TokenType>>> instructionPatternMap = {
        {InstructionType::R_TYPE,
         {{TokenType::REGISTER, TokenType::REGISTER, TokenType::REGISTER}}},

        {InstructionType::I_TYPE,
         {{TokenType::REGISTER, TokenType::REGISTER, TokenType::IMMEDIATE}}},

        {InstructionType::J_TYPE, {{TokenType::IMMEDIATE}, {TokenType::LABELREF}}}};

std::map<std::string, InstructionOp> instructionNameMap = {
        // Arithmetic and Logical Instructions
        {"add", {InstructionType::R_TYPE, 0x20}},
        {"addu", {InstructionType::R_TYPE, 0x21}},
        {"addi", {InstructionType::I_TYPE, 0x08}},
        {"addiu", {InstructionType::I_TYPE, 0x09}},
        {"and", {InstructionType::R_TYPE, 0x24}},
        {"andi", {InstructionType::I_TYPE, 0x0C}},
        {"div", {InstructionType::R_TYPE, 0x1A}},
        {"divu", {InstructionType::R_TYPE, 0x1B}},
        {"mult", {InstructionType::R_TYPE, 0x18}},
        {"multu", {InstructionType::R_TYPE, 0x19}},
        {"nor", {InstructionType::R_TYPE, 0x27}},
        {"or", {InstructionType::R_TYPE, 0x25}},
        {"ori", {InstructionType::I_TYPE, 0x0D}},
        {"sll", {InstructionType::R_TYPE, 0x00}},
        {"sllv", {InstructionType::R_TYPE, 0x04}},
        {"sra", {InstructionType::R_TYPE, 0x03}},
        {"srav", {InstructionType::R_TYPE, 0x07}},
        {"srl", {InstructionType::R_TYPE, 0x02}},
        {"srlv", {InstructionType::R_TYPE, 0x06}},
        {"sub", {InstructionType::R_TYPE, 0x22}},
        {"subu", {InstructionType::R_TYPE, 0x23}},
        {"xor", {InstructionType::R_TYPE, 0x26}},
        {"xori", {InstructionType::I_TYPE, 0x0E}},

        // Comparison Instructions
        {"slt", {InstructionType::R_TYPE, 0x2a}},
        {"sltu", {InstructionType::R_TYPE, 0x29}},
        {"slti", {InstructionType::I_TYPE, 0x0a}},
        {"sltiu", {InstructionType::I_TYPE, 0x09}},

        // Branch Instructions
        {"beq", {InstructionType::I_TYPE, 0x04}},
        {"bgtz", {InstructionType::I_TYPE, 0x07}},
        {"blez", {InstructionType::I_TYPE, 0x06}},
        {"bne", {InstructionType::I_TYPE, 0x05}},

        // Jump Instructions
        {"j", {InstructionType::J_TYPE, 0x02}},
        {"jal", {InstructionType::J_TYPE, 0x03}},
        {"jalr", {InstructionType::R_TYPE, 0x09}},
        {"jr", {InstructionType::R_TYPE, 0x08}},

        // Load Instructions
        {"lb", {InstructionType::I_TYPE, 0x20}},
        {"lbu", {InstructionType::I_TYPE, 0x24}},
        {"lh", {InstructionType::I_TYPE, 0x21}},
        {"lhu", {InstructionType::I_TYPE, 0x25}},
        {"lw", {InstructionType::I_TYPE, 0x23}},

        // Store Instructions
        {"sb", {InstructionType::I_TYPE, 0x28}},
        {"sh", {InstructionType::I_TYPE, 0x29}},
        {"sw", {InstructionType::I_TYPE, 0x2B}}};


InstructionOp nameToInstructionOp(const std::string& name) {
    if (instructionNameMap.contains(name))
        return instructionNameMap[name];
    throw std::runtime_error("Unknown instruction " + name);
}


void validateInstruction(const Token& instruction, const std::vector<Token>& args) {
    if (instruction.type != TokenType::INSTRUCTION)
        throw std::runtime_error("Expected instruction token, got " +
                                 std::to_string(static_cast<int>(instruction.type)));
    InstructionType instructionType = nameToInstructionOp(instruction.value).type;

    if (!instructionPatternMap.contains(instructionType))
        throw std::runtime_error("Unknown instruction type " +
                                 std::to_string(static_cast<int>(instructionType)));

    for (const std::vector<TokenType>& pattern : instructionPatternMap[instructionType])
        if (pattern.size() == args.size() &&
            std::equal(pattern.begin(), pattern.end(), args.begin()))
            return;

    throw std::runtime_error("Instruction " + instruction.value +
                             " arguments do not match any known patterns");
}
