//
// Created by matthew on 4/14/25.
//

#include "parser.h"

#include <algorithm>
#include <register.h>
#include <stdexcept>

#include "instruction.h"
#include "utils.h"


MemLayout Parser::parse(const std::vector<std::vector<Token>>& tokens) {
    MemLayout layout;

    MemSection currSection = MemSection::TEXT;
    layout[MemSection::TEXT] = {};

    // Resolve all labels before parsing instructions
    labelMap.populateLabelMap(tokens);

    for (size_t i = 0; i < tokens.size(); ++i) {
        // Skip empty lines
        if (tokens[i].empty())
            continue;

        try {
            parseLine(layout, currSection, tokens[i]);
        } catch (const std::runtime_error& e) {
            throw std::runtime_error("Error near line " + std::to_string(i + 1) + ": " + e.what());
        }
    }

    return layout;
}


void Parser::parseLine(MemLayout& layout, MemSection& currSection,
                       const std::vector<Token>& tokenLine) {
    // Get next open location in memory
    const uint32_t memLoc = memSectionOffset(currSection) + layout[currSection].size();

    const Token& firstToken = tokenLine[0];
    const std::vector unfilteredArgs(tokenLine.begin() + 1, tokenLine.end());
    std::vector<Token> args = filterTokenList(unfilteredArgs);

    switch (firstToken.type) {
        case TokenType::MEMDIRECTIVE: {
            currSection = nameToMemSection(firstToken.value);
            if (!layout.contains(currSection))
                layout[currSection] = {};
            break;
        }
        case TokenType::DIRECTIVE: {
            std::vector<std::byte> directiveBytes = parseDirective(firstToken, args);
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
        case TokenType::LABEL:
            break;
        default:
            throw std::runtime_error("Encountered unexpected token " + firstToken.value);
    }
}


std::vector<std::byte> Parser::parseDirective(const Token& dirToken,
                                              const std::vector<Token>& args) {
    const std::string dirName = dirToken.value;
    if (args.empty())
        throw std::runtime_error("Directive " + dirName + " expects at least one argument");

    std::vector<std::byte> bytes = {};

    // Return a byte vector containing each character of the string
    if (dirName == "asciiz" || dirName == "ascii") {
        if (args.size() != 1)
            throw std::runtime_error(dirName + " directive expects exactly one argument");
        if (args[0].type != TokenType::STRING)
            throw std::runtime_error(dirName + " directive expects a string argument");
        return stringToBytes(args[0].value, dirName == "asciiz", true);
    }
    // Return a word for each argument given
    if (dirName == "word") {
        for (const Token& arg : args) {
            if (arg.type != TokenType::IMMEDIATE)
                throw std::runtime_error("word directive expects immediate values as arguments");
            std::vector<std::byte> immediateBytes = intStringToBytes(arg.value);
            bytes.insert(bytes.end(), immediateBytes.begin(), immediateBytes.end());
        }
        return bytes;
    }

    if (dirName == "space") {
        if (args.size() != 1)
            throw std::runtime_error("space directive expects exactly one argument");
        if (args[0].type != TokenType::IMMEDIATE || !isSignedInteger(args[0].value) ||
            std::stoi(args[0].value) <= 0)
            throw std::runtime_error(dirName + " directive expects a positive integer argument");

        for (int i = 0; i < std::stoi(args[0].value); ++i)
            bytes.push_back(static_cast<std::byte>(0));
    }

    throw std::runtime_error("Unsupported directive " + dirName);
}


std::vector<std::byte> Parser::parseInstruction(const uint32_t loc, const Token& instrToken,
                                                std::vector<Token>& args) {
    RegisterFile regFile{};

    // Throw error if pattern for instruction is invalid
    validateInstruction(instrToken, args);

    // Resolve label references to their computed address values
    labelMap.resolveLabels(args);

    InstructionOp instructionOp = nameToInstructionOp(instrToken.value);
    std::vector<uint32_t> argCodes = {};
    // Parse the instruction argument token values into integers
    for (const Token& arg : args) {
        switch (arg.type) {
            case TokenType::IMMEDIATE:
                if (!isSignedInteger(arg.value))
                    throw std::runtime_error("Invalid integer " + arg.value);
                argCodes.push_back(static_cast<uint32_t>(std::stoi(arg.value)));
                break;
            case TokenType::REGISTER:
                if (isSignedInteger(arg.value) && std::stoi(arg.value) >= 0)
                    argCodes.push_back(std::stoi(arg.value));
                else
                    argCodes.push_back(regFile.indexFromName(arg.value));
                break;
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
        case InstructionType::R_TYPE_S:
            return parseRTypeInstruction(0, argCodes[0], 0, 0, opFuncCode);
        case InstructionType::I_TYPE_T_S_I:
            return parseITypeInstruction(loc, opFuncCode, argCodes[0], argCodes[1],
                                         static_cast<int32_t>(argCodes[2]));
        case InstructionType::I_TYPE_S_T_I:
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
        const int32_t offset = (immediate - static_cast<int32_t>(loc) - 8) >> 2;
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


std::vector<std::byte> Parser::parsePseudoInstruction(const uint32_t loc,
                                                      const std::string& instructionName,
                                                      const std::vector<Token>& args) {
    // li $t0, imm -> addiu $t0, $zero, imm
    if (instructionName == "li") {
        std::vector modifiedArgs = {args[0], {TokenType::REGISTER, "zero"}, args[1]};
        return parseInstruction(loc, {TokenType::INSTRUCTION, "addiu"}, modifiedArgs);
    }
    // la $t0, label -> lui $at, upperAddr; ori $t0, $at, lowerAddr
    if (instructionName == "la") {
        const unsigned int upperBytes = (std::stoi(args[1].value) & 0xFFFF0000) >> 16;
        const unsigned int lowerBytes = std::stoi(args[1].value) & 0x0000FFFF;

        std::vector<Token> modifiedArgs = {{TokenType::REGISTER, "at"},
                                           {TokenType::IMMEDIATE, std::to_string(upperBytes)}};
        std::vector<std::byte> luiBytes =
                parseInstruction(loc, {TokenType::INSTRUCTION, "lui"}, modifiedArgs);

        modifiedArgs = {args[0],
                        {TokenType::REGISTER, "at"},
                        {TokenType::IMMEDIATE, std::to_string(lowerBytes)}};
        std::vector<std::byte> oriBytes =
                parseInstruction(loc, {TokenType::INSTRUCTION, "ori"}, modifiedArgs);
        luiBytes.insert(luiBytes.end(), oriBytes.begin(), oriBytes.end());
        return luiBytes;
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


std::vector<std::byte> Parser::parseBranchPseudoInstruction(const uint32_t loc, const Token& reg1,
                                                            const Token& reg2, const Token& label,
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

    modifiedArgs = {{TokenType::REGISTER, "at"}, {TokenType::REGISTER, "zero"}, label};
    std::vector<std::byte> branchBytes;
    // Swap branch instruction when including equal in comparrison or not
    if (checkEq)
        branchBytes = parseInstruction(loc, {TokenType::INSTRUCTION, "beq"}, modifiedArgs);
    else
        branchBytes = parseInstruction(loc, {TokenType::INSTRUCTION, "bne"}, modifiedArgs);

    sltBytes.insert(sltBytes.end(), branchBytes.begin(), branchBytes.end());
    return sltBytes;
}


void Parser::validateInstruction(const Token& instruction, const std::vector<Token>& args) {

    switch (nameToInstructionOp(instruction.value).type) {
        case InstructionType::R_TYPE_D_T_S:
        case InstructionType::R_TYPE_D_S_T:
            if (!tokenTypeMatch({TokenType::REGISTER, TokenType::REGISTER, TokenType::REGISTER},
                                args))
                throw std::runtime_error("Invalid format for R-Type instruction " +
                                         instruction.value);
            break;
        case InstructionType::R_TYPE_D_T_H:
            if (!tokenTypeMatch({TokenType::REGISTER, TokenType::REGISTER, TokenType::IMMEDIATE},
                                args))
                throw std::runtime_error("Invalid format for R-Type instruction " +
                                         instruction.value);
            break;
        case InstructionType::I_TYPE_S_T_I:
        case InstructionType::I_TYPE_T_S_I:
            if (!tokenTypeMatch({TokenType::REGISTER, TokenType::REGISTER, TokenType::IMMEDIATE},
                                args))
                throw std::runtime_error("Invalid format for I-Type instruction " +
                                         instruction.value);
            break;
        case InstructionType::I_TYPE_T_I:
            if (!tokenTypeMatch({TokenType::REGISTER, TokenType::IMMEDIATE}, args))
                throw std::runtime_error("Invalid format for I-Type instruction " +
                                         instruction.value);
            break;
        case InstructionType::R_TYPE_S_T:
            if (!tokenTypeMatch({TokenType::REGISTER, TokenType::REGISTER}, args))
                throw std::runtime_error("Invalid format for R-Type instruction " +
                                         instruction.value);
            break;
        case InstructionType::R_TYPE_S:
            if (!tokenTypeMatch({TokenType::REGISTER}, args))
                throw std::runtime_error("Invalid format for R-Type instruction " +
                                         instruction.value);
            break;
        case InstructionType::J_TYPE_L:
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


void Parser::validatePseudoInstruction(const Token& instruction, const std::vector<Token>& args) {
    std::vector<std::string> branchPseudoInstrs = {"blt", "bgt", "ble", "bge"};
    std::vector<std::string> branchZeroPseudoInstrs = {"bltz", "bgtz", "blez", "bgez"};
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
    if (std::ranges::find(branchZeroPseudoInstrs, instructionName) !=
                branchZeroPseudoInstrs.end() &&
        !tokenTypeMatch({TokenType::REGISTER, TokenType::LABELREF}, args))
        throw std::runtime_error("Invalid format for instruction " + instruction.value);
}
