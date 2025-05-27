//
// Created by matthew on 4/14/25.
//

#include "parser/parser.h"

#include <../include/interpreter/register.h>
#include <algorithm>
#include <stdexcept>

#include "exceptions.h"
#include "parser/directive.h"
#include "parser/instruction.h"
#include "utils.h"


MemLayout Parser::parse(const std::vector<SourceLine>& tokenLines) {
    MemLayout layout;

    MemSection currSection = MemSection::TEXT;
    layout[MemSection::TEXT] = {};
    try {
        // Resolve all labels before parsing instructions
        labelMap.populateLabelMap(tokenLines);
    } catch (const std::runtime_error& e) {
        throw MasmSyntaxError(std::string(e.what()));
    }

    for (const auto& tokenLine : tokenLines) {
        // Skip empty lines
        if (tokenLine.tokens.empty())
            continue;
        try {
            parseLine(layout, currSection, tokenLine);
        } catch (const std::runtime_error& e) {
            throw MasmSyntaxError(e.what(), tokenLine.lineno);
        }
    }

    return layout;
}


void Parser::parseLine(MemLayout& layout, MemSection& currSection, const SourceLine& tokenLine) {
    // Get next open location in memory
    uint32_t memLoc = memSectionOffset(currSection) + layout[currSection].size();

    const Token& firstToken = tokenLine.tokens[0];
    const std::vector unfilteredArgs(tokenLine.tokens.begin() + 1, tokenLine.tokens.end());
    std::vector<Token> args = filterTokenList(unfilteredArgs);

    switch (firstToken.type) {
        case TokenType::SEC_DIRECTIVE: {
            currSection = nameToMemSection(firstToken.value);
            if (!layout.contains(currSection))
                layout[currSection] = {};
            break;
        }
        case TokenType::ALLOC_DIRECTIVE: {
            std::vector<std::byte> directiveBytes = parseAllocDirective(memLoc, firstToken, args);
            layout[currSection].insert(layout[currSection].end(), directiveBytes.begin(),
                                       directiveBytes.end());
            break;
        }
        case TokenType::INSTRUCTION: {
            const std::vector<std::byte> instrBytes = parseInstruction(memLoc, firstToken, args);
            layout[currSection].insert(layout[currSection].end(), instrBytes.begin(),
                                       instrBytes.end());
            break;
        }
        case TokenType::LABEL_DEF:
            break;
        default:
            throw std::runtime_error("Encountered unexpected token " + firstToken.value);
    }
}


std::vector<std::byte> Parser::parseInstruction(uint32_t& loc, const Token& instrToken,
                                                std::vector<Token>& args) {

    // Throw error if pattern for instruction is invalid
    validateInstruction(instrToken, args);

    RegisterFile regFile{};

    // Resolve label references to their computed address values
    labelMap.resolveLabels(args);

    InstructionOp instructionOp = nameToInstructionOp(instrToken.value);
    std::vector<uint32_t> argCodes = {};
    // Parse the instruction argument token values into integers
    for (const Token& arg : args) {
        switch (arg.type) {
            case TokenType::IMMEDIATE:
                argCodes.push_back(stoui32(arg.value));
                break;
            case TokenType::REGISTER: {
                if (isSignedInteger(arg.value))
                    argCodes.push_back(stoui32(arg.value));
                else
                    argCodes.push_back(regFile.indexFromName(arg.value));
                break;
            }
            default:
                // Should never be reached
                throw std::runtime_error("Invalid argument type " +
                                         std::to_string(static_cast<int>(arg.type)));
        }
    }

    const uint32_t opFuncCode = static_cast<uint32_t>(instructionOp.opFuncCode);
    // Parse integer arguments into a single instruction word
    switch (instructionOp.type) {
        case InstructionType::R_TYPE_D_S_T:
            return parseRTypeInstruction(argCodes[0], argCodes[1], argCodes[2], 0, opFuncCode);
        case InstructionType::R_TYPE_D_T_S:
            return parseRTypeInstruction(argCodes[0], argCodes[2], argCodes[1], 0, opFuncCode);
        case InstructionType::R_TYPE_D_T_H:
            return parseRTypeInstruction(argCodes[0], 0, argCodes[1], argCodes[2], opFuncCode);
        case InstructionType::R_TYPE_D:
            return parseRTypeInstruction(argCodes[0], 0, 0, 0, opFuncCode);
        case InstructionType::R_TYPE_S:
            return parseRTypeInstruction(0, argCodes[0], 0, 0, opFuncCode);
        case InstructionType::I_TYPE_T_S_I:
            return parseITypeInstruction(loc, opFuncCode, argCodes[0], argCodes[1],
                                         static_cast<int32_t>(argCodes[2]));
        case InstructionType::I_TYPE_S_T_L:
            // Instructions where rs comes before rt in the binary encoding
            return parseITypeInstruction(loc, opFuncCode, argCodes[1], argCodes[0],
                                         static_cast<int32_t>(argCodes[2]));
        case InstructionType::I_TYPE_T_I:
            // Location not needed for short I-Type instructions
            return parseITypeInstruction(0, opFuncCode, argCodes[0], 0,
                                         static_cast<int32_t>(argCodes[1]));
        case InstructionType::R_TYPE_S_T:
            return parseRTypeInstruction(0, argCodes[0], argCodes[1], 0, opFuncCode);
        case InstructionType::J_TYPE_L:
            return parseJTypeInstruction(opFuncCode, argCodes[0]);
        case InstructionType::SYSCALL:
            return parseSyscallInstruction();
        case InstructionType::PSEUDO:
            return parsePseudoInstruction(loc, instrToken.value, args);
    }
    // Should never be reached
    throw std::runtime_error("Unknown instruction type " +
                             std::to_string(static_cast<int>(instructionOp.type)));
}


std::vector<std::byte> Parser::parseRTypeInstruction(const uint32_t rd, const uint32_t rs,
                                                     const uint32_t rt, const uint32_t shamt,
                                                     const uint32_t funct) {

    // Combine fields into 32-bit instruction code
    const uint32_t instruction = (0 & 0x3F) << 26 | (rs & 0x1F) << 21 | (rt & 0x1F) << 16 |
                                 (rd & 0x1F) << 11 | (shamt & 0x1F) << 6 | funct & 0x3F;
    return i32ToBEByte(instruction);
}


std::vector<std::byte> Parser::parseITypeInstruction(const uint32_t loc, const uint32_t opcode,
                                                     const uint32_t rt, const uint32_t rs,
                                                     int32_t immediate) {

    // Modify immediate values to be relative to the location of the current instruction
    std::vector branchOpCodes = {static_cast<uint32_t>(InstructionCode::BEQ),
                                 static_cast<uint32_t>(InstructionCode::BNE)};
    if (std::ranges::find(branchOpCodes, opcode) != branchOpCodes.end()) {
        // Branch instructions are offset by 4 bytes so divide by 4
        const int32_t offset = (immediate - static_cast<int32_t>(loc) - 4) >> 2;
        if (offset < -32768 || offset > 32767)
            throw std::runtime_error("Branch instruction offset out of range");
        immediate = static_cast<int32_t>(offset);
    }

    // Combine fields into 32-bit instruction code
    const uint32_t instruction =
            (opcode & 0x3F) << 26 | (rs & 0x1F) << 21 | (rt & 0x1F) << 16 | immediate & 0xFFFF;
    return i32ToBEByte(instruction);
}


std::vector<std::byte> Parser::parseJTypeInstruction(const uint32_t opcode,
                                                     const uint32_t address) {

    // Combine fields into 32-bit instruction code (address shifted by 2 to byte align to 4)
    const uint32_t instruction = (opcode & 0x3F) << 26 | (address & 0x3FFFFFF) >> 2;
    return i32ToBEByte(instruction);
}


std::vector<std::byte> Parser::parseSyscallInstruction() { return i32ToBEByte(12); }


std::vector<std::byte> Parser::parsePseudoInstruction(uint32_t& loc,
                                                      const std::string& instructionName,
                                                      const std::vector<Token>& args) {
    // li $t0, imm -> addiu $t0, $zero, imm
    if (instructionName == "li") {
        std::vector modifiedArgs = {args[0], {TokenType::REGISTER, "zero"}, args[1]};
        return parseInstruction(loc, {TokenType::INSTRUCTION, "addiu"}, modifiedArgs);
    }
    // la $t0, label -> lui $at, upperAddr; ori $t0, $at, lowerAddr
    if (instructionName == "la") {
        const uint32_t value = stoui32(args[1].value);
        const unsigned int upperBytes = (value & 0xFFFF0000) >> 16;
        const unsigned int lowerBytes = value & 0x0000FFFF;

        std::vector<Token> modifiedArgs = {{TokenType::REGISTER, "at"},
                                           {TokenType::IMMEDIATE, std::to_string(upperBytes)}};
        std::vector<std::byte> luiBytes =
                parseInstruction(loc, {TokenType::INSTRUCTION, "lui"}, modifiedArgs);

        loc += 4; // Increment location since we have added an instruction

        modifiedArgs = {args[0],
                        {TokenType::REGISTER, "at"},
                        {TokenType::IMMEDIATE, std::to_string(lowerBytes)}};
        std::vector<std::byte> oriBytes =
                parseInstruction(loc, {TokenType::INSTRUCTION, "ori"}, modifiedArgs);
        luiBytes.insert(luiBytes.end(), oriBytes.begin(), oriBytes.end());
        return luiBytes;
    }
    // move $tx, $ty -> addu $tx, $ty, $zero
    if (instructionName == "move") {
        std::vector modifiedArgs = {args[0], {TokenType::REGISTER, "zero"}, args[1]};
        return parseInstruction(loc, {TokenType::INSTRUCTION, "addu"}, modifiedArgs);
    }
    // mul $tx, $ty, $tz -> mult $ty, $tz; mflo $tx
    if (instructionName == "mul") {
        std::vector<Token> modifiedArgs = {args[1], args[2]};
        std::vector<std::byte> multBytes =
                parseInstruction(loc, {TokenType::INSTRUCTION, "mult"}, modifiedArgs);

        loc += 4; // Increment location since we have added an instruction

        modifiedArgs = {{TokenType::REGISTER, args[0].value}};
        std::vector<std::byte> mfloBytes =
                parseInstruction(loc, {TokenType::INSTRUCTION, "mflo"}, modifiedArgs);
        multBytes.insert(multBytes.end(), mfloBytes.begin(), mfloBytes.end());
        return multBytes;
    }
    // nop -> sll $zero, $zero, 0
    if (instructionName == "nop") {
        std::vector<Token> modifiedArgs = {{TokenType::REGISTER, "zero"},
                                           {TokenType::REGISTER, "zero"},
                                           {TokenType::IMMEDIATE, "0"}};
        return parseInstruction(loc, {TokenType::INSTRUCTION, "sll"}, modifiedArgs);
    }

    // bxx $tx, $tx, label -> slt $at, $tx, $tx; bxx $at, $zero, label
    std::vector<std::string> branchPseudoInstrs = {"blt", "bgt", "ble", "bge"};
    if (std::ranges::find(branchPseudoInstrs, instructionName) != branchPseudoInstrs.end()) {
        if (instructionName == branchPseudoInstrs[0])
            return parseBranchPseudoInstruction(loc, args[0], args[1], args[2], true, false);
        if (instructionName == branchPseudoInstrs[1])
            return parseBranchPseudoInstruction(loc, args[0], args[1], args[2], false, false);
        if (instructionName == branchPseudoInstrs[2])
            return parseBranchPseudoInstruction(loc, args[0], args[1], args[2], false, true);
        if (instructionName == branchPseudoInstrs[3])
            return parseBranchPseudoInstruction(loc, args[0], args[1], args[2], true, true);
    }

    // bxxz $tx, label -> slt $at, $zero, $tx; bxx $at, $zero, label
    const Token regZero = {TokenType::REGISTER, "zero"};
    std::vector<std::string> branchZeroPseudoInstrs = {"bltz", "bgtz", "blez", "bgez"};
    if (std::ranges::find(branchZeroPseudoInstrs, instructionName) !=
        branchZeroPseudoInstrs.end()) {
        if (instructionName == branchZeroPseudoInstrs[0])
            return parseBranchPseudoInstruction(loc, args[0], regZero, args[1], true, false);
        if (instructionName == branchZeroPseudoInstrs[1])
            return parseBranchPseudoInstruction(loc, args[0], regZero, args[1], false, false);
        if (instructionName == branchZeroPseudoInstrs[2])
            return parseBranchPseudoInstruction(loc, args[0], regZero, args[1], false, true);
        if (instructionName == branchZeroPseudoInstrs[3])
            return parseBranchPseudoInstruction(loc, args[0], regZero, args[1], true, true);
    }

    // Should never be reached
    throw std::runtime_error("Unknown pseudo instruction " + instructionName);
}


std::vector<std::byte> Parser::parseBranchPseudoInstruction(uint32_t& loc, const Token& reg1,
                                                            const Token& reg2,
                                                            const Token& labelAddr,
                                                            const bool checkLt,
                                                            const bool checkEq) {
    std::vector<Token> modifiedArgs;
    // Swap argument order when checking for less than or greater than relationship
    if (checkLt)
        modifiedArgs = {{TokenType::REGISTER, "at"}, reg1, reg2};
    else
        modifiedArgs = {{TokenType::REGISTER, "at"}, reg2, reg1};

    std::vector<std::byte> sltBytes =
            parseInstruction(loc, {TokenType::INSTRUCTION, "slt"}, modifiedArgs);

    loc += 4; // Increment location since we have added an instruction

    // Recover address from parsed label
    const std::string labelName = labelMap.lookupLabel(std::stoi(labelAddr.value));
    modifiedArgs = {{TokenType::REGISTER, "at"},
                    {TokenType::REGISTER, "zero"},
                    {TokenType::LABEL_REF, labelName}};
    std::vector<std::byte> branchBytes;
    // Swap branch instruction when including equal in comparrison or not
    if (checkEq)
        branchBytes = parseInstruction(loc, {TokenType::INSTRUCTION, "beq"}, modifiedArgs);
    else
        branchBytes = parseInstruction(loc, {TokenType::INSTRUCTION, "bne"}, modifiedArgs);

    sltBytes.insert(sltBytes.end(), branchBytes.begin(), branchBytes.end());
    return sltBytes;
}
