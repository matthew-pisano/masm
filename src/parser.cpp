//
// Created by matthew on 4/14/25.
//

#include "parser.h"

#include <algorithm>
#include <register.h>
#include <stdexcept>

#include "instruction.h"
#include "utils.h"


/**
 * Converts a 32-bit integer to a vector of bytes in big-endian order
 * @param i32 The 32-bit integer to convert
 * @return A vector of bytes representing the integer in big-endian order
 */
std::vector<std::byte> i32ToBEByte(const uint32_t i32) {
    // Break the instruction into 4 bytes (big-endian)
    std::vector<std::byte> bytes(4);
    bytes[0] = static_cast<std::byte>(i32 >> 24 & 0xFF); // Most significant byte
    bytes[1] = static_cast<std::byte>(i32 >> 16 & 0xFF);
    bytes[2] = static_cast<std::byte>(i32 >> 8 & 0xFF);
    bytes[3] = static_cast<std::byte>(i32 & 0xFF); // Least significant byte
    return bytes;
}


void Parser::resolveLabels(std::vector<Token>& instructionArgs) {
    for (Token& arg : instructionArgs)
        if (arg.type == TokenType::LABELREF) {
            if (!labelMap.contains(arg.value))
                throw std::runtime_error("Unknown label " + arg.value);
            arg = {TokenType::IMMEDIATE, std::to_string(labelMap[arg.value])};
        }
}


void Parser::populateLabelMap(const std::vector<std::vector<Token>>& tokens) {
    MemSection currSection = MemSection::TEXT;
    std::map<MemSection, uint32_t> memSizes = {{currSection, 0}};

    for (const std::vector<Token>& line : tokens) {
        if (line.empty())
            continue;

        const Token& firstToken = line[0];
        const std::vector unfilteredArgs(line.begin() + 1, line.end());
        std::vector<Token> args = filterTokenList(unfilteredArgs);
        switch (firstToken.type) {
            case TokenType::MEMDIRECTIVE: {
                currSection = nameToMemSection(firstToken.value);
                if (!memSizes.contains(currSection))
                    memSizes[currSection] = 0;
                break;
            }
            case TokenType::DIRECTIVE: {
                memSizes[currSection] += parseDirective(firstToken, args).size();
                break;
            }
            case TokenType::INSTRUCTION:
                // Get size of instruction from map without parsing
                memSizes[currSection] += nameToInstructionOp(firstToken.value).size;
                break;
            case TokenType::LABEL: {
                if (labelMap.contains(firstToken.value))
                    throw std::runtime_error("Duplicate label " + firstToken.value);
                // Assign label to the following byte allocation plus the section offset
                labelMap[firstToken.value] = memSectionOffset(currSection) + memSizes[currSection];
                break;
            }
            default:
                break;
        }
    }
}


std::vector<std::byte> Parser::parseDirective(const Token& dirToken,
                                              const std::vector<Token>& args) {
    const std::string dirName = dirToken.value;
    if (args.empty())
        throw std::runtime_error("Directive " + dirName + " expects at least one argument");

    std::vector<std::byte> bytes = {};

    // Return a byte vector containing each character of the string
    if (dirName == "asciiz") {
        if (args.size() > 1)
            throw std::runtime_error(".asciiz expects exactly one argument");
        if (args[0].type != TokenType::STRING)
            throw std::runtime_error(".asciiz expects a string argument");
        return stringToBytes(args[0].value);
    }
    // Return a word for each argument given
    if (dirName == "word") {
        for (const Token& arg : args) {
            if (arg.type != TokenType::IMMEDIATE)
                throw std::runtime_error(".word expects immediate values as arguments");
            std::vector<std::byte> immediateBytes = intStringToBytes(arg.value);
            bytes.insert(bytes.end(), immediateBytes.begin(), immediateBytes.end());
        }
        return bytes;
    }

    throw std::runtime_error("Unsupported directive " + dirName);
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
    std::vector<uint32_t> branchOpCodes = {0x04, 0x07, 0x06, 0x05, 0x01};
    if (std::ranges::find(branchOpCodes, opcode) != branchOpCodes.end()) {
        // Branch instructions are offset by 4 bytes so divide by 4
        const int32_t offset = (immediate - static_cast<int32_t>(loc) - 8) / 4;
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
                                                      std::vector<Token>& args) {
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

std::vector<std::byte> Parser::parseInstruction(const uint32_t loc, const Token& instrToken,
                                                std::vector<Token>& args) {
    RegisterFile regFile{};

    // Throw error if pattern for instruction is invalid
    validateInstruction(instrToken, args);

    // Resolve label references to their computed address values
    resolveLabels(args);

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
                throw std::runtime_error("Invalid argument type " +
                                         std::to_string(static_cast<int>(arg.type)));
        }
    }

    const uint32_t opFuncCode = static_cast<uint32_t>(instructionOp.opFuncCode);
    // Parse integer arguments into a single instruction word
    switch (instructionOp.type) {
        case InstructionType::R_TYPE:
            return parseRTypeInstruction(argCodes[0], argCodes[1], argCodes[2], 0, opFuncCode);
        case InstructionType::I_TYPE:
            return parseITypeInstruction(loc, opFuncCode, argCodes[0], argCodes[1],
                                         static_cast<int32_t>(argCodes[2]));
        case InstructionType::SWAPPED_I_TYPE:
            // Instructions where rs comes before rt in the binary encoding
            return parseITypeInstruction(loc, opFuncCode, argCodes[1], argCodes[0],
                                         static_cast<int32_t>(argCodes[2]));
        case InstructionType::SHORT_I_TYPE:
            // Location not needed for short I-Type instructions
            return parseITypeInstruction(0, opFuncCode, argCodes[0], 0,
                                         static_cast<int32_t>(argCodes[1]));
        case InstructionType::J_TYPE:
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
            throw std::runtime_error("Encountered unknown token type during parsing for token " +
                                     firstToken.value);
    }
}


MemLayout Parser::parse(const std::vector<std::vector<Token>>& tokens) {
    MemLayout layout;

    MemSection currSection = MemSection::TEXT;
    layout[MemSection::TEXT] = {};

    // Resolve all labels before parsing instructions
    populateLabelMap(tokens);

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
