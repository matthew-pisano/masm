//
// Created by matthew on 4/15/25.
//

#include "instruction.h"

#include <map>
#include <stdexcept>

#include "utils.h"


/**
 * A mapping between instruction names and their associated properties
 */
std::map<std::string, InstructionOp> instructionNameMap = {
        // Arithmetic and Logical Instructions
        {"add", {InstructionType::R_TYPE, InstructionCode::ADD, 4}},
        {"addu", {InstructionType::R_TYPE, InstructionCode::ADDU, 4}},
        {"addi", {InstructionType::I_TYPE, InstructionCode::ADDI, 4}},
        {"addiu", {InstructionType::I_TYPE, InstructionCode::ADDIU, 4}},
        {"and", {InstructionType::R_TYPE, InstructionCode::AND, 4}},
        {"andi", {InstructionType::I_TYPE, InstructionCode::ANDI, 4}},
        {"div", {InstructionType::R_TYPE, InstructionCode::DIV, 4}},
        {"divu", {InstructionType::R_TYPE, InstructionCode::DIVU, 4}},
        {"mult", {InstructionType::R_TYPE, InstructionCode::MULT, 4}},
        {"multu", {InstructionType::R_TYPE, InstructionCode::MULTU, 4}},
        {"nor", {InstructionType::R_TYPE, InstructionCode::NOR, 4}},
        {"or", {InstructionType::R_TYPE, InstructionCode::OR, 4}},
        {"ori", {InstructionType::I_TYPE, InstructionCode::ORI, 4}},
        {"sll", {InstructionType::R_TYPE, InstructionCode::SLL, 4}},
        {"sllv", {InstructionType::R_TYPE, InstructionCode::SLLV, 4}},
        {"sra", {InstructionType::R_TYPE, InstructionCode::SRA, 4}},
        {"srav", {InstructionType::R_TYPE, InstructionCode::SRAV, 4}},
        {"srl", {InstructionType::R_TYPE, InstructionCode::SRL, 4}},
        {"srlv", {InstructionType::R_TYPE, InstructionCode::SRLV, 4}},
        {"sub", {InstructionType::R_TYPE, InstructionCode::SUB, 4}},
        {"subu", {InstructionType::R_TYPE, InstructionCode::SUBU, 4}},
        {"xor", {InstructionType::R_TYPE, InstructionCode::XOR, 4}},
        {"xori", {InstructionType::I_TYPE, InstructionCode::XORI, 4}},

        // Comparison Instructions
        {"slt", {InstructionType::R_TYPE, InstructionCode::SLT, 4}},
        {"sltu", {InstructionType::R_TYPE, InstructionCode::SLTU, 4}},
        {"slti", {InstructionType::I_TYPE, InstructionCode::SLTI, 4}},
        {"sltiu", {InstructionType::I_TYPE, InstructionCode::SLTIU, 4}},

        // Branch Instructions
        {"beq", {InstructionType::SWAPPED_I_TYPE, InstructionCode::BEQ, 4}},
        {"bne", {InstructionType::SWAPPED_I_TYPE, InstructionCode::BNE, 4}},

        // Jump Instructions
        {"j", {InstructionType::J_TYPE, InstructionCode::J, 4}},
        {"jal", {InstructionType::J_TYPE, InstructionCode::JAL, 4}},
        {"jalr", {InstructionType::R_TYPE, InstructionCode::JALR, 4}},
        {"jr", {InstructionType::R_TYPE, InstructionCode::JR, 4}},

        // Load Instructions
        {"lb", {InstructionType::I_TYPE, InstructionCode::LB, 4}},
        {"lbu", {InstructionType::I_TYPE, InstructionCode::LBU, 4}},
        {"lh", {InstructionType::I_TYPE, InstructionCode::LH, 4}},
        {"lhu", {InstructionType::I_TYPE, InstructionCode::LHU, 4}},
        {"lw", {InstructionType::I_TYPE, InstructionCode::LW, 4}},

        // Store Instructions
        {"sb", {InstructionType::I_TYPE, InstructionCode::SB, 4}},
        {"sh", {InstructionType::I_TYPE, InstructionCode::SH, 4}},
        {"sw", {InstructionType::I_TYPE, InstructionCode::SW, 4}},

        // Syscall
        {"syscall", {InstructionType::SYSCALL, InstructionCode::SYSCALL, 4}},

        // Pseudo Instructions
        {"li", {InstructionType::PSEUDO, InstructionCode::LI, 4}},
        {"la", {InstructionType::PSEUDO, InstructionCode::LA, 8}},
        {"blt", {InstructionType::PSEUDO, InstructionCode::BLT, 8}},
        {"bgt", {InstructionType::PSEUDO, InstructionCode::BGT, 8}},
        {"bge", {InstructionType::PSEUDO, InstructionCode::BGE, 8}},
        {"ble", {InstructionType::PSEUDO, InstructionCode::BLE, 8}},

        // Remapped Instructions (real instructions remapped to more simple instructions)
        {"bgtz", {InstructionType::PSEUDO, InstructionCode::BGTZ, 8}},
        {"blez", {InstructionType::PSEUDO, InstructionCode::BLEZ, 8}},
        {"bltz", {InstructionType::PSEUDO, InstructionCode::BLTZ, 8}},
        {"bgez", {InstructionType::PSEUDO, InstructionCode::BGEZ, 8}},
        {"lui", {InstructionType::PSEUDO, InstructionCode::LUI, 4}},

};


bool operator==(const uint32_t lhs, InstructionCode code) {
    return lhs == static_cast<uint32_t>(code);
}


InstructionOp nameToInstructionOp(const std::string& name) {
    if (instructionNameMap.contains(name))
        return instructionNameMap[name];
    throw std::runtime_error("Unknown instruction " + name);
}


void validateInstruction(const Token& instruction, const std::vector<Token>& args) {

    switch (nameToInstructionOp(instruction.value).type) {
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
    const std::string instructionName = instruction.value;

    if (instructionName == "li" &&
        !tokenTypeMatch({TokenType::REGISTER, TokenType::IMMEDIATE}, args))
        throw std::runtime_error("Invalid format for instruction " + instruction.value);
    if (instructionName == "la" &&
        !tokenTypeMatch({TokenType::REGISTER, TokenType::LABELREF}, args))
        throw std::runtime_error("Invalid format for instruction " + instruction.value);
    if (instructionName == "lui" &&
        !tokenTypeMatch({TokenType::REGISTER, TokenType::IMMEDIATE}, args))
        throw std::runtime_error("Invalid format for instruction " + instruction.value);
    if (std::ranges::find(branchPseudoInstrs, instructionName) != branchPseudoInstrs.end() &&
        !tokenTypeMatch({TokenType::REGISTER, TokenType::REGISTER, TokenType::LABELREF}, args))
        throw std::runtime_error("Invalid format for instruction " + instruction.value);
}
