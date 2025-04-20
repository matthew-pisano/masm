//
// Created by matthew on 4/14/25.
//

#include "parser.h"

#include <algorithm>
#include <register.h>
#include <stdexcept>

#include "instruction.h"
#include "utils.h"


void Parser::resolveLabels(std::vector<Token>& instructionArgs) {
    for (Token& arg : instructionArgs)
        if (arg.type == TokenType::LABELREF) {
            if (!labelMap.contains(arg.value))
                throw std::runtime_error("Unknown label " + arg.value);
            arg = {TokenType::IMMEDIATE, std::to_string(labelMap[arg.value])};
        }
}


void Parser::populateLabels(const std::vector<std::vector<Token>>& tokens) {
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
                std::vector<uint8_t> directiveBytes = parseDirective(firstToken, args);
                memSizes[currSection] += directiveBytes.size();
                break;
            }
            case TokenType::INSTRUCTION: {
                memSizes[currSection] += nameToInstructionOp(firstToken.value).size;
                break;
            }
            case TokenType::LABEL: {
                if (labelMap.contains(firstToken.value))
                    throw std::runtime_error("Duplicate label " + firstToken.value);
                labelMap[firstToken.value] = memSectionOffset(currSection) + memSizes[currSection];
                break;
            }
            default: {
                break;
            }
        }
    }
}


std::vector<uint8_t> Parser::parseDirective(const Token& dirToken, const std::vector<Token>& args) {
    const std::string dirName = dirToken.value;
    if (args.empty())
        throw std::runtime_error("Directive " + dirName + " expects at least one argument");

    std::vector<uint8_t> bytes = {};

    if (dirName == "asciiz") {
        if (args.size() > 1)
            throw std::runtime_error(".asciiz expects exactly one argument");
        if (args[0].type != TokenType::STRING)
            throw std::runtime_error(".asciiz expects a string argument");
        return stringToBytes(args[0].value);
    }
    if (dirName == "word") {
        for (const Token& arg : args) {
            if (arg.type != TokenType::IMMEDIATE)
                throw std::runtime_error(".word expects immediate values as arguments");
            std::vector<uint8_t> immediateBytes = intStringToBytes(arg.value);
            bytes.insert(bytes.end(), immediateBytes.begin(), immediateBytes.end());
        }
        return bytes;
    }

    throw std::runtime_error("Unsupported directive " + dirName);
}


std::vector<uint8_t> Parser::parseRTypeInstruction(const uint32_t rd, const uint32_t rs,
                                                   const uint32_t rt, const uint32_t shamt,
                                                   const uint32_t funct) {

    // Combine fields into 32-bit instruction code
    const uint32_t instruction = (0 & 0x3F) << 26 | (rs & 0x1F) << 21 | (rt & 0x1F) << 16 |
                                 (rd & 0x1F) << 11 | (shamt & 0x1F) << 6 | funct & 0x3F;

    // Break the instruction into 4 bytes (big-endian)
    std::vector<uint8_t> bytes(4);
    bytes[0] = instruction >> 24 & 0xFF; // Most significant byte
    bytes[1] = instruction >> 16 & 0xFF;
    bytes[2] = instruction >> 8 & 0xFF;
    bytes[3] = instruction & 0xFF; // Least significant byte

    return bytes;
}


std::vector<uint8_t> Parser::parseITypeInstruction(const uint32_t loc, const uint32_t opcode,
                                                   const uint32_t rt, const uint32_t rs,
                                                   int32_t immediate) {

    // Modify immediate values to be relative to the location of the current instruction
    std::vector<uint32_t> branchOpCodes = {0x04, 0x07, 0x06, 0x05, 0x01};
    if (std::ranges::find(branchOpCodes, opcode) != branchOpCodes.end()) {
        // Branch instructions are offset by 4 bytes so divide by 4
        const int32_t offset = (immediate - loc - 8) / 4;
        if (offset < -32768 || offset > 32767)
            throw std::runtime_error("Branch instruction offset out of range");
        immediate = static_cast<int32_t>(offset);
    }

    // Combine fields into 32-bit instruction code
    const uint32_t instruction =
            (opcode & 0x3F) << 26 | (rs & 0x1F) << 21 | (rt & 0x1F) << 16 | immediate & 0xFFFF;

    // Break the instruction into 4 bytes (big-endian)
    std::vector<uint8_t> bytes(4);
    bytes[0] = instruction >> 24 & 0xFF; // Most significant byte
    bytes[1] = instruction >> 16 & 0xFF;
    bytes[2] = instruction >> 8 & 0xFF;
    bytes[3] = instruction & 0xFF; // Least significant byte

    return bytes;
}


std::vector<uint8_t> Parser::parseJTypeInstruction(const uint32_t opcode, const uint32_t address) {

    // Combine fields into 32-bit instruction code (address shifted by 2 to byte align to 4)
    const uint32_t instruction = (opcode & 0x3F) << 26 | (address & 0x3FFFFFF) >> 2;

    // Break the instruction into 4 bytes (big-endian)
    std::vector<uint8_t> bytes(4);
    bytes[0] = instruction >> 24 & 0xFF; // Most significant byte
    bytes[1] = instruction >> 16 & 0xFF;
    bytes[2] = instruction >> 8 & 0xFF;
    bytes[3] = instruction & 0xFF; // Least significant byte

    return bytes;
}


std::vector<uint8_t> Parser::parseSyscallInstruction() { return {0x00, 0x00, 0x00, 0x0c}; }


std::vector<uint8_t> Parser::parsePseudoInstruction(const uint32_t loc,
                                                    const std::string& instructionName,
                                                    std::vector<Token>& args) {

    if (instructionName == "li") {
        std::vector modifiedArgs = {args[0], {TokenType::REGISTER, "zero"}, args[1]};
        return parseInstruction(loc, {TokenType::INSTRUCTION, "addiu"}, modifiedArgs);
    }
    if (instructionName == "la") {
        const unsigned int upperBytes = (std::stoi(args[1].value) & 0xFFFF0000) >> 16;
        const unsigned int lowerBytes = std::stoi(args[1].value) & 0x0000FFFF;
        std::vector<Token> modifiedArgs = {{TokenType::REGISTER, "at"},
                                           {TokenType::IMMEDIATE, std::to_string(upperBytes)}};
        std::vector<uint8_t> luiBytes =
                parseInstruction(loc, {TokenType::INSTRUCTION, "lui"}, modifiedArgs);

        modifiedArgs = {args[0],
                        {TokenType::REGISTER, "at"},
                        {TokenType::IMMEDIATE, std::to_string(lowerBytes)}};
        std::vector<uint8_t> oriBytes =
                parseInstruction(loc, {TokenType::INSTRUCTION, "ori"}, modifiedArgs);
        luiBytes.insert(luiBytes.end(), oriBytes.begin(), oriBytes.end());
        return luiBytes;
    }
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


std::vector<uint8_t> Parser::parseBranchPseudoInstruction(const uint32_t loc, const Token& reg1,
                                                          const Token& reg2, const Token& label,
                                                          const bool checkLt, const bool checkEq) {
    std::vector<Token> modifiedArgs;
    if (checkLt)
        modifiedArgs = {{TokenType::REGISTER, "at"}, reg1, reg2};
    else
        modifiedArgs = {{TokenType::REGISTER, "at"}, reg2, reg1};

    std::vector<uint8_t> sltBytes =
            parseInstruction(loc, {TokenType::INSTRUCTION, "slt"}, modifiedArgs);
    std::vector<uint8_t> branchBytes;
    modifiedArgs = {{TokenType::REGISTER, "at"}, {TokenType::REGISTER, "zero"}, label};
    if (checkEq)
        branchBytes = parseInstruction(loc, {TokenType::INSTRUCTION, "beq"}, modifiedArgs);
    else
        branchBytes = parseInstruction(loc, {TokenType::INSTRUCTION, "bne"}, modifiedArgs);

    sltBytes.insert(sltBytes.end(), branchBytes.begin(), branchBytes.end());
    return sltBytes;
}

std::vector<uint8_t> Parser::parseInstruction(const uint32_t loc, const Token& instrToken,
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

    // Parse integer arguments into a single instruction word
    switch (instructionOp.type) {
        case InstructionType::R_TYPE:
            return parseRTypeInstruction(argCodes[0], argCodes[1], argCodes[2], 0,
                                         instructionOp.opFuncCode);
        case InstructionType::I_TYPE:
            return parseITypeInstruction(loc, instructionOp.opFuncCode, argCodes[0], argCodes[1],
                                         argCodes[2]);
        case InstructionType::SWAPPED_I_TYPE:
            // Instructions where rs comes before rt in the binary encoding
            return parseITypeInstruction(loc, instructionOp.opFuncCode, argCodes[1], argCodes[0],
                                         argCodes[2]);
        case InstructionType::SHORT_I_TYPE:
            // Location not needed for short I-Type instructions
            return parseITypeInstruction(0, instructionOp.opFuncCode, argCodes[0], 0, argCodes[1]);

        case InstructionType::J_TYPE:
            return parseJTypeInstruction(instructionOp.opFuncCode, argCodes[0]);

        case InstructionType::SYSCALL:
            return parseSyscallInstruction();

        case InstructionType::PSEUDO:
            return parsePseudoInstruction(loc, instrToken.value, args);
    }
    // Should never be reached
    throw std::runtime_error("Unknown instruction type " +
                             std::to_string(static_cast<int>(instructionOp.type)));
}


MemLayout Parser::parse(const std::vector<std::vector<Token>>& tokens) {
    MemLayout memory;

    // Resolve all labels before parsing instructions
    populateLabels(tokens);

    MemSection currSection = MemSection::TEXT;
    memory[currSection] = {};

    for (const std::vector<Token>& line : tokens) {
        if (line.empty())
            continue;

        const uint32_t memLoc = memSectionOffset(currSection) + memory[currSection].size();
        const Token& firstToken = line[0];
        const std::vector unfilteredArgs(line.begin() + 1, line.end());
        std::vector<Token> args = filterTokenList(unfilteredArgs);

        switch (firstToken.type) {
            case TokenType::MEMDIRECTIVE: {
                currSection = nameToMemSection(firstToken.value);
                if (!memory.contains(currSection))
                    memory[currSection] = {};
                break;
            }
            case TokenType::DIRECTIVE: {
                std::vector<uint8_t> directiveBytes = parseDirective(firstToken, args);
                memory[currSection].insert(memory[currSection].end(), directiveBytes.begin(),
                                           directiveBytes.end());
                break;
            }
            case TokenType::INSTRUCTION: {
                const std::vector<uint8_t> instructionBytes =
                        parseInstruction(memLoc, firstToken, args);
                memory[currSection].insert(memory[currSection].end(), instructionBytes.begin(),
                                           instructionBytes.end());
                break;
            }
            case TokenType::LABEL: {
                break;
            }
            default: {
                throw std::runtime_error(
                        "Encountered unknown token type during parsing for token " +
                        firstToken.value);
            }
        }
    }

    return memory;
}
