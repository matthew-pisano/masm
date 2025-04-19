//
// Created by matthew on 4/15/25.
//

#ifndef INSTRUCTION_H
#define INSTRUCTION_H
#include <cstdint>
#include "tokenizer.h"

enum class InstructionType { R_TYPE, I_TYPE, J_TYPE, SHORT_I_TYPE, SYSCALL, PSEUDO };

struct InstructionOp {
    InstructionType type;
    uint8_t opFuncCode;
};


InstructionOp nameToInstructionOp(const std::string& name);

void validateInstruction(const Token& instruction, const std::vector<Token>& args);

void validatePseudoInstruction(const Token& instruction, const std::vector<Token>& args);

#endif // INSTRUCTION_H
