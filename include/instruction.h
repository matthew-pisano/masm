//
// Created by matthew on 4/15/25.
//

#ifndef INSTRUCTION_H
#define INSTRUCTION_H
#include <cstdint>
#include "tokenizer.h"


/**
 * Class representing all valid instruction types and sub-types
 */
enum class InstructionType {
    R_TYPE,
    I_TYPE,
    J_TYPE,
    SHORT_I_TYPE,
    SWAPPED_I_TYPE,
    SYSCALL,
    PSEUDO
};


/**
 * Class Detailing the details of a specific instruction
 */
struct InstructionOp {
    InstructionType type;
    uint8_t opFuncCode;
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
 * @throw runtime_error when the arguments for a pseudo instruction do not match its accepted values
 */
void validatePseudoInstruction(const Token& instruction, const std::vector<Token>& args);

#endif // INSTRUCTION_H
