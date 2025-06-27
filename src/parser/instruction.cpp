//
// Created by matthew on 4/15/25.
//

#include "parser/instruction.h"

#include <algorithm>
#include <map>
#include <stdexcept>

#include "utils.h"


/**
 * A mapping between instruction names and their associated properties
 */
std::map<std::string, InstructionOp> instructionNameMap = {
        // Arithmetic and Logical Instructions
        {"add", {InstructionType::R_TYPE_D_S_T, InstructionCode::ADD, 4}},
        {"addu", {InstructionType::R_TYPE_D_S_T, InstructionCode::ADDU, 4}},
        {"addi", {InstructionType::I_TYPE_T_S_I, InstructionCode::ADDI, 4}},
        {"addiu", {InstructionType::I_TYPE_T_S_I, InstructionCode::ADDIU, 4}},
        {"and", {InstructionType::R_TYPE_D_S_T, InstructionCode::AND, 4}},
        {"andi", {InstructionType::I_TYPE_T_S_I, InstructionCode::ANDI, 4}},
        {"div", {InstructionType::R_TYPE_S_T, InstructionCode::DIV, 4}},
        {"divu", {InstructionType::R_TYPE_S_T, InstructionCode::DIVU, 4}},
        {"mfhi", {InstructionType::R_TYPE_D, InstructionCode::MFHI, 4}},
        {"mthi", {InstructionType::R_TYPE_S, InstructionCode::MTHI, 4}},
        {"mflo", {InstructionType::R_TYPE_D, InstructionCode::MFLO, 4}},
        {"mtlo", {InstructionType::R_TYPE_S, InstructionCode::MTLO, 4}},
        {"mult", {InstructionType::R_TYPE_S_T, InstructionCode::MULT, 4}},
        {"multu", {InstructionType::R_TYPE_S_T, InstructionCode::MULTU, 4}},
        {"nor", {InstructionType::R_TYPE_D_S_T, InstructionCode::NOR, 4}},
        {"or", {InstructionType::R_TYPE_D_S_T, InstructionCode::OR, 4}},
        {"ori", {InstructionType::I_TYPE_T_S_I, InstructionCode::ORI, 4}},
        {"sll", {InstructionType::R_TYPE_D_T_H, InstructionCode::SLL, 4}},
        {"sllv", {InstructionType::R_TYPE_D_T_S, InstructionCode::SLLV, 4}},
        {"sra", {InstructionType::R_TYPE_D_T_H, InstructionCode::SRA, 4}},
        {"srav", {InstructionType::R_TYPE_D_T_S, InstructionCode::SRAV, 4}},
        {"srl", {InstructionType::R_TYPE_D_T_H, InstructionCode::SRL, 4}},
        {"srlv", {InstructionType::R_TYPE_D_T_S, InstructionCode::SRLV, 4}},
        {"sub", {InstructionType::R_TYPE_D_S_T, InstructionCode::SUB, 4}},
        {"subu", {InstructionType::R_TYPE_D_S_T, InstructionCode::SUBU, 4}},
        {"xor", {InstructionType::R_TYPE_D_S_T, InstructionCode::XOR, 4}},
        {"xori", {InstructionType::I_TYPE_T_S_I, InstructionCode::XORI, 4}},

        // Comparison Instructions
        {"slt", {InstructionType::R_TYPE_D_S_T, InstructionCode::SLT, 4}},
        {"sltu", {InstructionType::R_TYPE_D_S_T, InstructionCode::SLTU, 4}},
        {"slti", {InstructionType::I_TYPE_T_S_I, InstructionCode::SLTI, 4}},
        {"sltiu", {InstructionType::I_TYPE_T_S_I, InstructionCode::SLTIU, 4}},

        // Branch Instructions
        {"beq", {InstructionType::I_TYPE_S_T_L, InstructionCode::BEQ, 4}},
        {"bne", {InstructionType::I_TYPE_S_T_L, InstructionCode::BNE, 4}},

        // Jump Instructions
        {"j", {InstructionType::J_TYPE_L, InstructionCode::J, 4}},
        {"jal", {InstructionType::J_TYPE_L, InstructionCode::JAL, 4}},
        {"jalr", {InstructionType::R_TYPE_S, InstructionCode::JALR, 4}},
        {"jr", {InstructionType::R_TYPE_S, InstructionCode::JR, 4}},

        // Load Instructions
        {"lb", {InstructionType::I_TYPE_T_S_I, InstructionCode::LB, 4}},
        {"lbu", {InstructionType::I_TYPE_T_S_I, InstructionCode::LBU, 4}},
        {"lh", {InstructionType::I_TYPE_T_S_I, InstructionCode::LH, 4}},
        {"lhu", {InstructionType::I_TYPE_T_S_I, InstructionCode::LHU, 4}},
        {"lw", {InstructionType::I_TYPE_T_S_I, InstructionCode::LW, 4}},
        {"lui", {InstructionType::I_TYPE_T_I, InstructionCode::LUI, 4}},

        // Store Instructions
        {"sb", {InstructionType::I_TYPE_T_S_I, InstructionCode::SB, 4}},
        {"sh", {InstructionType::I_TYPE_T_S_I, InstructionCode::SH, 4}},
        {"sw", {InstructionType::I_TYPE_T_S_I, InstructionCode::SW, 4}},

        // Remapped Instructions (supported by the ISA, but remapped for convenience of parsing)
        {"beqz", {InstructionType::PSEUDO, InstructionCode::BEQZ, 4}},
        {"bnez", {InstructionType::PSEUDO, InstructionCode::BNEZ, 4}},
        {"bgtz", {InstructionType::PSEUDO, InstructionCode::BGTZ, 8}},
        {"blez", {InstructionType::PSEUDO, InstructionCode::BLEZ, 8}},
        {"bltz", {InstructionType::PSEUDO, InstructionCode::BLTZ, 8}},
        {"bgez", {InstructionType::PSEUDO, InstructionCode::BGEZ, 8}},

        // Syscall
        {"syscall", {InstructionType::SYSCALL, InstructionCode::SYSCALL, 4}},

        // Co-Processor 0 Instructions
        {"mfc0", {InstructionType::CP0_TYPE_T_D, InstructionCode::MFC0, 4}},
        {"mtc0", {InstructionType::CP0_TYPE_T_D, InstructionCode::MTC0, 4}},

        // Eret
        {"eret", {InstructionType::ERET, InstructionCode::ERET, 4}},

        // Co Processor 1 (Floating Point) Instructions
        // Arithmetic Instructions
        {"abs.s", {InstructionType::CP1_TYPE_SP_D_S, InstructionCode::FP_ABS, 4}},
        {"abs.d", {InstructionType::CP1_TYPE_DP_D_S, InstructionCode::FP_ABS, 4}},
        {"add.s", {InstructionType::CP1_TYPE_SP_D_S_T, InstructionCode::FP_ADD, 4}},
        {"add.d", {InstructionType::CP1_TYPE_DP_D_S_T, InstructionCode::FP_ADD, 4}},
        {"div.s", {InstructionType::CP1_TYPE_SP_D_S_T, InstructionCode::FP_DIV, 4}},
        {"div.d", {InstructionType::CP1_TYPE_DP_D_S_T, InstructionCode::FP_DIV, 4}},
        {"mul.s", {InstructionType::CP1_TYPE_SP_D_S_T, InstructionCode::FP_MUL, 4}},
        {"mul.d", {InstructionType::CP1_TYPE_DP_D_S_T, InstructionCode::FP_MUL, 4}},
        {"neg.s", {InstructionType::CP1_TYPE_SP_D_S, InstructionCode::FP_NEG, 4}},
        {"neg.d", {InstructionType::CP1_TYPE_DP_D_S, InstructionCode::FP_NEG, 4}},
        {"sqrt.s", {InstructionType::CP1_TYPE_SP_D_S, InstructionCode::FP_SQRT, 4}},
        {"sqrt.d", {InstructionType::CP1_TYPE_DP_D_S, InstructionCode::FP_SQRT, 4}},
        {"sub.s", {InstructionType::CP1_TYPE_SP_D_S_T, InstructionCode::FP_SUB, 4}},
        {"sub.d", {InstructionType::CP1_TYPE_DP_D_S_T, InstructionCode::FP_SUB, 4}},

        // Comparison Instructions
        {"c.eq.s", {InstructionType::CP1_TYPE_SP_S_T_C, InstructionCode::FP_C_EQ, 4}},
        {"c.eq.d", {InstructionType::CP1_TYPE_DP_S_T_C, InstructionCode::FP_C_EQ, 4}},
        {"c.lt.s", {InstructionType::CP1_TYPE_SP_S_T_C, InstructionCode::FP_C_LT, 4}},
        {"c.lt.d", {InstructionType::CP1_TYPE_DP_S_T_C, InstructionCode::FP_C_LT, 4}},
        {"c.le.s", {InstructionType::CP1_TYPE_SP_S_T_C, InstructionCode::FP_C_LE, 4}},
        {"c.le.d", {InstructionType::CP1_TYPE_DP_S_T_C, InstructionCode::FP_C_LE, 4}},

        // Branch Instructions
        {"bc1f", {InstructionType::CP1_TYPE_L, InstructionCode::FP_BC1F, 4}},
        {"bc1t", {InstructionType::CP1_TYPE_L, InstructionCode::FP_BC1T, 4}},

        // Conversion Instructions
        {"cvt.d.s", {InstructionType::CP1_TYPE_SP_D_S, InstructionCode::FP_CVT_D, 4}},
        {"cvt.s.d", {InstructionType::CP1_TYPE_DP_D_S, InstructionCode::FP_CVT_S, 4}},

        // Load Instructions
        {"lwc1", {InstructionType::CP1_TYPE_T_S_I, InstructionCode::FP_LWC1, 4}},
        {"ldc1", {InstructionType::CP1_TYPE_T_S_I, InstructionCode::FP_LDC1, 4}},

        // Store Instructions
        {"swc1", {InstructionType::CP1_TYPE_T_S_I, InstructionCode::FP_SWC1, 4}},
        {"sdc1", {InstructionType::CP1_TYPE_T_S_I, InstructionCode::FP_SDC1, 4}},

        // Move Instructions
        {"mfc1", {InstructionType::CP1_TYPE_T_S, InstructionCode::FP_MFC1, 4}},
        {"mtc1", {InstructionType::CP1_TYPE_T_S, InstructionCode::FP_MTC1, 4}},
        {"mov.s", {InstructionType::CP1_TYPE_SP_D_S, InstructionCode::FP_MOV, 4}},
        {"mov.d", {InstructionType::CP1_TYPE_DP_D_S, InstructionCode::FP_MOV, 4}},

        // Pseudo Instructions
        {"li", {InstructionType::PSEUDO, InstructionCode::PSEUDO, 4}},
        {"la", {InstructionType::PSEUDO, InstructionCode::PSEUDO, 8}},
        {"move", {InstructionType::PSEUDO, InstructionCode::PSEUDO, 4}},
        {"mul", {InstructionType::PSEUDO, InstructionCode::PSEUDO, 8}},
        {"nop", {InstructionType::PSEUDO, InstructionCode::PSEUDO, 4}},
        {"subi", {InstructionType::PSEUDO, InstructionCode::PSEUDO, 4}},
        {"blt", {InstructionType::PSEUDO, InstructionCode::PSEUDO, 8}},
        {"bgt", {InstructionType::PSEUDO, InstructionCode::PSEUDO, 8}},
        {"bge", {InstructionType::PSEUDO, InstructionCode::PSEUDO, 8}},
        {"ble", {InstructionType::PSEUDO, InstructionCode::PSEUDO, 8}},
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
        // Core CPU Instructions
        case InstructionType::R_TYPE_D_T_S:
        case InstructionType::R_TYPE_D_S_T:
            if (!tokenCategoryMatch(
                        {TokenCategory::REGISTER, TokenCategory::REGISTER, TokenCategory::REGISTER},
                        args))
                throw std::runtime_error("Invalid format for R-Type instruction " +
                                         instruction.value);
            break;
        case InstructionType::R_TYPE_D_T_H:
            if (!tokenCategoryMatch({TokenCategory::REGISTER, TokenCategory::REGISTER,
                                     TokenCategory::IMMEDIATE},
                                    args))
                throw std::runtime_error("Invalid format for R-Type instruction " +
                                         instruction.value);
            break;
        case InstructionType::I_TYPE_S_T_L:
            if (!tokenCategoryMatch({TokenCategory::REGISTER, TokenCategory::REGISTER,
                                     TokenCategory::LABEL_REF},
                                    args))
                throw std::runtime_error("Invalid format for I-Type instruction " +
                                         instruction.value);
            break;
        case InstructionType::I_TYPE_T_S_I:
            if (!tokenCategoryMatch({TokenCategory::REGISTER, TokenCategory::REGISTER,
                                     TokenCategory::IMMEDIATE},
                                    args))
                throw std::runtime_error("Invalid format for I-Type instruction " +
                                         instruction.value);
            break;
        case InstructionType::I_TYPE_T_I:
            if (!tokenCategoryMatch({TokenCategory::REGISTER, TokenCategory::IMMEDIATE}, args))
                throw std::runtime_error("Invalid format for I-Type instruction " +
                                         instruction.value);
            break;
        case InstructionType::R_TYPE_S_T:
            if (!tokenCategoryMatch({TokenCategory::REGISTER, TokenCategory::REGISTER}, args))
                throw std::runtime_error("Invalid format for R-Type instruction " +
                                         instruction.value);
            break;
        case InstructionType::R_TYPE_D:
        case InstructionType::R_TYPE_S:
            if (!tokenCategoryMatch({TokenCategory::REGISTER}, args))
                throw std::runtime_error("Invalid format for R-Type instruction " +
                                         instruction.value);
            break;
        case InstructionType::J_TYPE_L:
            if (!tokenCategoryMatch({TokenCategory::LABEL_REF}, args))
                throw std::runtime_error("Invalid format for J-Type instruction " +
                                         instruction.value);
            break;
        case InstructionType::SYSCALL:
            if (!tokenCategoryMatch({}, args))
                throw std::runtime_error("Invalid format for Syscall");
            break;

        // Co-Processor 0 Instructions
        case InstructionType::CP0_TYPE_T_D:
            if (!tokenCategoryMatch({TokenCategory::REGISTER, TokenCategory::REGISTER}, args))
                throw std::runtime_error("Invalid format for Co-Processor 0 instruction " +
                                         instruction.value);
            break;
        case InstructionType::ERET:
            if (!tokenCategoryMatch({}, args))
                throw std::runtime_error("Invalid format for Eret instruction");
            break;

        // Co-Processor 1 Instructions (Floating Point)
        case InstructionType::CP1_TYPE_SP_D_S:
        case InstructionType::CP1_TYPE_DP_D_S:
        case InstructionType::CP1_TYPE_SP_S_T_C:
        case InstructionType::CP1_TYPE_DP_S_T_C:
        case InstructionType::CP1_TYPE_T_S:
            if (!tokenCategoryMatch({TokenCategory::REGISTER, TokenCategory::REGISTER}, args))
                throw std::runtime_error("Invalid format for Co-Processor 1 instruction " +
                                         instruction.value);
            break;
        case InstructionType::CP1_TYPE_SP_D_S_T:
        case InstructionType::CP1_TYPE_DP_D_S_T:
            if (!tokenCategoryMatch(
                        {TokenCategory::REGISTER, TokenCategory::REGISTER, TokenCategory::REGISTER},
                        args))
                throw std::runtime_error("Invalid format for Co-Processor 1 instruction " +
                                         instruction.value);
            break;
        case InstructionType::CP1_TYPE_T_S_I:
            if (!tokenCategoryMatch({TokenCategory::REGISTER, TokenCategory::REGISTER,
                                     TokenCategory::IMMEDIATE},
                                    args))
                throw std::runtime_error("Invalid format for Co-Processor 1 instruction " +
                                         instruction.value);
            break;
        case InstructionType::CP1_TYPE_L:
            if (!tokenCategoryMatch({TokenCategory::LABEL_REF}, args))
                throw std::runtime_error("Invalid format for Co-Processor 1 instruction " +
                                         instruction.value);
            break;
        default:
            // Should never be reached
            throw std::runtime_error("Unknown instruction " + instruction.value);
    }
}


void validatePseudoInstruction(const Token& instruction, const std::vector<Token>& args) {
    std::vector<std::string> branchPseudoInstrs = {"blt", "bgt", "ble", "bge"};
    std::vector<std::string> branchZeroPseudoInstrs = {"bltz", "bgtz", "blez",
                                                       "bgez", "beqz", "bnez"};
    const std::string instructionName = instruction.value;

    if (instructionName == "li" &&
        !tokenCategoryMatch({TokenCategory::REGISTER, TokenCategory::IMMEDIATE}, args))
        throw std::runtime_error("Invalid format for instruction " + instruction.value);
    if (instructionName == "la" &&
        !tokenCategoryMatch({TokenCategory::REGISTER, TokenCategory::LABEL_REF}, args) &&
        !tokenCategoryMatch({TokenCategory::REGISTER, TokenCategory::IMMEDIATE}, args))
        throw std::runtime_error("Invalid format for instruction " + instruction.value);
    if (instructionName == "lui" &&
        !tokenCategoryMatch({TokenCategory::REGISTER, TokenCategory::IMMEDIATE}, args))
        throw std::runtime_error("Invalid format for instruction " + instruction.value);
    if (instructionName == "move" &&
        !tokenCategoryMatch({TokenCategory::REGISTER, TokenCategory::REGISTER}, args))
        throw std::runtime_error("Invalid format for instruction " + instruction.value);
    if (instructionName == "mul" &&
        !tokenCategoryMatch(
                {TokenCategory::REGISTER, TokenCategory::REGISTER, TokenCategory::REGISTER}, args))
        throw std::runtime_error("Invalid format for instruction " + instruction.value);
    if (instructionName == "nop" && !tokenCategoryMatch({}, args))
        throw std::runtime_error("Invalid format for instruction " + instruction.value);
    if (instructionName == "subi" &&
        !tokenCategoryMatch(
                {TokenCategory::REGISTER, TokenCategory::REGISTER, TokenCategory::IMMEDIATE}, args))
        throw std::runtime_error("Invalid format for instruction " + instruction.value);
    if (std::ranges::find(branchPseudoInstrs, instructionName) != branchPseudoInstrs.end() &&
        !tokenCategoryMatch(
                {TokenCategory::REGISTER, TokenCategory::REGISTER, TokenCategory::LABEL_REF}, args))
        throw std::runtime_error("Invalid format for instruction " + instruction.value);
    if (std::ranges::find(branchZeroPseudoInstrs, instructionName) !=
                branchZeroPseudoInstrs.end() &&
        !tokenCategoryMatch({TokenCategory::REGISTER, TokenCategory::LABEL_REF}, args))
        throw std::runtime_error("Invalid format for instruction " + instruction.value);
}


bool isInstruction(const std::string& token) { return instructionNameMap.contains(token); }
