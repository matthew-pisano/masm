//
// Created by matthew on 4/15/25.
//

#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <cstdint>
#include "tokenizer/tokenizer.h"


/**
 * The Op or Func code associated with an instruction
 */
enum class InstructionCode {
    // Arithmetic and Logical Instructions
    ADD = 0x20,
    ADDU = 0x21,
    ADDI = 0x08,
    ADDIU = 0x09,
    AND = 0x24,
    ANDI = 0x0c,
    DIV = 0x1a,
    DIVU = 0x1b,
    MFHI = 0x10,
    MFLO = 0x12,
    MTHI = 0x11,
    MTLO = 0x13,
    MULT = 0x18,
    MULTU = 0x19,
    NOR = 0x27,
    OR = 0x25,
    ORI = 0x0d,
    SLL = 0x00,
    SLLV = 0x04,
    SRA = 0x03,
    SRAV = 0x07,
    SRL = 0x02,
    SRLV = 0x06,
    SUB = 0x22,
    SUBU = 0x23,
    XOR = 0x26,
    XORI = 0x0e,

    // Comparison Instructions
    SLT = 0x2a,
    SLTU = 0x29,
    SLTI = 0x0a,
    SLTIU = 0x0b,

    // Branch Instructions
    BEQ = 0x04,
    BNE = 0x05,

    // Jump Instructions
    J = 0x02,
    JAL = 0x03,
    JALR = 0x09,
    JR = 0x08,

    // Load Instructions
    LB = 0x20,
    LBU = 0x24,
    LH = 0x21,
    LHU = 0x25,
    LW = 0x23,
    LUI = 0x0f,

    // Store Instructions
    SB = 0x28,
    SH = 0x29,
    SW = 0x2b,

    // Remapped Instructions (supported by the ISA, but remapped for convenience of parsing)
    BGTZ = 0x07,
    BLEZ = 0x06,
    BLTZ = 0x07,
    BGEZ = 0x01,

    // Syscall
    SYSCALL = 0x00,

    // Co Processor 0 Instructions
    MFC0 = 0x00,
    MTC0 = 0x04,

    // Eret
    ERET = 0x00,

    // Co Processor 1 (Floating Point) Instructions
    // Arithmetic Instructions
    FP_ABS = 0x05,
    FP_ADD = 0x00,
    FP_DIV = 0x03,
    FP_MUL = 0x02,
    FP_NEG = 0x07,
    FP_SQRT = 0x04,
    FP_SUB = 0x01,

    // Comparison Instructions
    FP_C_EQ = 0x02,
    FP_C_LT = 0x0c,
    FP_C_LE = 0x0e,

    // Branch Instructions
    FP_BC1F = 0x00,
    FP_BC1T = 0x01,

    // Conversion Instructions
    FP_CVT_D = 0x21,
    FP_CVT_S = 0x20,

    // Load Instructions
    FP_LDC1 = 0x35,
    FP_LWC1 = 0x31,

    // Store Instructions
    FP_SDC1 = 0x3d,
    FP_SWC1 = 0x39,

    // Move Instructions
    FP_MFC1 = 0x00,
    FP_MTC1 = 0x04,
    FP_MOV = 0x06,

    // Instruction Code for Pseudo Instructions
    PSEUDO = 0x00,
};


bool operator==(uint32_t lhs, InstructionCode code);


/**
 * Class representing all valid instruction types and subtypes (used for mapping arguments)
 */
enum class InstructionType {
    // D -> Destination Register
    // S -> First Source Register
    // T -> Second Source Register
    // H -> Shift Amount (shamt)
    // I -> Immediate Value
    // L -> Label Reference

    // SP -> Single Precision Floating Point
    // DP -> Double Precision Floating Point
    // C -> Condition

    R_TYPE_D_S_T, // R-Type
    R_TYPE_D_T_H, // R-Type with shamt
    R_TYPE_D, // R-Type with only destination register
    R_TYPE_S_T, // R-Type with 2 source registers
    R_TYPE_D_T_S, // R-Type with source registers swapped
    R_TYPE_S, // R-Type with 1 source register
    I_TYPE_T_S_I, // I-Type
    I_TYPE_T_I, // I-Type with 1 register
    I_TYPE_S_T_L, // I-Type with source registers swapped and label
    J_TYPE_L, // J-Type
    SYSCALL, // Syscall

    ERET, // Eret instruction
    CP0_TYPE_T_D, // Co-Processor 0 Type (Move from/to CP0)

    CP1_TYPE_SP_D_S,
    CP1_TYPE_DP_D_S,
    CP1_TYPE_SP_D_S_T,
    CP1_TYPE_DP_D_S_T,
    CP1_TYPE_L,
    CP1_TYPE_SP_S_T_C,
    CP1_TYPE_DP_S_T_C,
    CP1_TYPE_T_S,
    CP1_TYPE_T_S_I,

    PSEUDO // Pseudo instruction
};


/**
 * Class holding the details of a specific instruction
 */
struct InstructionOp {
    /**
     * The type of instruction, which determines how the arguments are interpreted
     */
    InstructionType type;

    /**
     * The function code or operation code associated with the instruction
     */
    InstructionCode opFuncCode;

    /**
     * The size of the instruction in bytes, used for memory allocation and alignment.
     * This is usually 4 bytes for most instructions, but can vary for pseudo instructions.
     */
    uint8_t size;
};


/**
 * Fetches the instruction associated with its name
 * @param name The name of the instruction
 * @return The instruction representation
 * @throw runtime_error When an unknown instruction is named
 */
InstructionOp nameToInstructionOp(const std::string& name);


/**
 * Ensures that the arguments of an instruction match an expected pattern of token types
 * @param instruction The instruction token
 * @param args A list of arguments following the given instruction
 * @throw runtime_error when the arguments for an instruction do not match its accepted values
 */
void validateInstruction(const Token& instruction, const std::vector<Token>& args);


/**
 * Ensures that the arguments of a pseudo instruction match an expected pattern of token types
 * @param instruction The pseudo instruction token
 * @param args A list of arguments following the given pseudo instruction
 * @throw runtime_error when the arguments for a pseudo instruction do not match its accepted
 * values
 */
void validatePseudoInstruction(const Token& instruction, const std::vector<Token>& args);


/**
 * Checks if the given token is an instruction name
 * @param token The token to check
 * @return True if the token is an instruction name, false otherwise
 */
bool isInstruction(const std::string& token);


#endif // INSTRUCTION_H
