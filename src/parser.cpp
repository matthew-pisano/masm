//
// Created by matthew on 4/14/25.
//

#include "parser.h"

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


std::vector<uint8_t> Parser::parseRTypeInstruction(const uint32_t rs, const uint32_t rt,
                                                   const uint32_t rd, const uint32_t shamt,
                                                   const uint32_t funct) {

    // Combine fields into 32-bit instruction code
    const uint32_t instruction = (0 & 0x3F) << 26 | (rs & 0x1F) << 21 | (rd & 0x1F) << 16 |
                                 (rt & 0x1F) << 11 | (shamt & 0x1F) << 6 | funct & 0x3F;

    // Break the instruction into 4 bytes (big-endian)
    std::vector<uint8_t> bytes(4);
    bytes[0] = instruction >> 24 & 0xFF; // Most significant byte
    bytes[1] = instruction >> 16 & 0xFF;
    bytes[2] = instruction >> 8 & 0xFF;
    bytes[3] = instruction & 0xFF; // Least significant byte

    return bytes;
}


std::vector<uint8_t> Parser::parseITypeInstruction(const uint32_t opcode, const uint32_t rs,
                                                   const uint32_t rt, const uint32_t immediate) {

    // Combine fields into 32-bit instruction code
    const uint32_t instruction =
            (opcode & 0x3F) << 26 | (rt & 0x1F) << 21 | (rs & 0x1F) << 16 | immediate & 0xFFFF;

    // Break the instruction into 4 bytes (big-endian)
    std::vector<uint8_t> bytes(4);
    bytes[0] = instruction >> 24 & 0xFF; // Most significant byte
    bytes[1] = instruction >> 16 & 0xFF;
    bytes[2] = instruction >> 8 & 0xFF;
    bytes[3] = instruction & 0xFF; // Least significant byte

    return bytes;
}


std::vector<uint8_t> Parser::parseShortITypeInstruction(const uint32_t opcode, const uint32_t rt,
                                                        const uint32_t immediate) {

    return parseITypeInstruction(opcode, rt, 0, immediate);
}


std::vector<uint8_t> Parser::parseJTypeInstruction(const uint32_t opcode, const uint32_t address) {

    // Combine fields into 32-bit instruction code
    const uint32_t instruction = (opcode & 0x3F) << 26 | (address & 0x3FFFFFF);

    // Break the instruction into 4 bytes (big-endian)
    std::vector<uint8_t> bytes(4);
    bytes[0] = instruction >> 24 & 0xFF; // Most significant byte
    bytes[1] = instruction >> 16 & 0xFF;
    bytes[2] = instruction >> 8 & 0xFF;
    bytes[3] = instruction & 0xFF; // Least significant byte

    return bytes;
}


std::vector<uint8_t> Parser::parsePseudoInstruction(const std::string& instructionName,
                                                    std::vector<Token>& args) {
    // Resolve label references to their computed address values
    resolveLabels(args);

    if (instructionName == "li") {
        std::vector modifiedArgs = {args[0], {TokenType::REGISTER, "0"}, args[1]};
        return parseInstruction({TokenType::INSTRUCTION, "addiu"}, modifiedArgs);
    }
    if (instructionName == "la") {
        const unsigned int upperBytes = (std::stoi(args[1].value) & 0xFFFF0000) >> 16;
        const unsigned int lowerBytes = std::stoi(args[1].value) & 0x0000FFFF;
        std::vector<Token> modifiedArgs = {{TokenType::REGISTER, "at"},
                                           {TokenType::IMMEDIATE, std::to_string(upperBytes)}};
        std::vector<uint8_t> luiBytes =
                parseInstruction({TokenType::INSTRUCTION, "lui"}, modifiedArgs);

        modifiedArgs = {args[0],
                        {TokenType::REGISTER, "at"},
                        {TokenType::IMMEDIATE, std::to_string(lowerBytes)}};
        std::vector<uint8_t> oriBytes =
                parseInstruction({TokenType::INSTRUCTION, "ori"}, modifiedArgs);
        luiBytes.insert(luiBytes.end(), oriBytes.begin(), oriBytes.end());
        return luiBytes;
    }

    throw std::runtime_error("Unknown pseudo instruction " + instructionName);
}

std::vector<uint8_t> Parser::parseInstruction(const Token& instrToken, std::vector<Token>& args) {
    RegisterFile regFile{};

    // Throw error if pattern for instruction is invalid
    validateInstruction(instrToken, args);

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
            case TokenType::LABELREF:
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
            return parseITypeInstruction(instructionOp.opFuncCode, argCodes[0], argCodes[1],
                                         argCodes[2]);
        case InstructionType::SHORT_I_TYPE:
            return parseShortITypeInstruction(instructionOp.opFuncCode, argCodes[0], argCodes[1]);

        case InstructionType::J_TYPE:
            return parseJTypeInstruction(instructionOp.opFuncCode, argCodes[0]);

        case InstructionType::PSEUDO:
            return parsePseudoInstruction(instrToken.value, args);
    }
    // Should never be reached
    throw std::runtime_error("Unknown instruction type " +
                             std::to_string(static_cast<int>(instructionOp.type)));
}


MemLayout Parser::parse(const std::vector<std::vector<Token>>& tokens) {
    MemSection currSection = MemSection::TEXT;
    memory[currSection] = {};
    std::vector<std::string> pendingLabels = {};

    for (const std::vector<Token>& line : tokens) {
        if (line.empty())
            continue;

        const Token& firstToken = line[0];
        const std::vector unfilteredArgs(line.begin() + 1, line.end());
        std::vector<Token> args = filterTokenList(unfilteredArgs);
        size_t initialMemSecSize = memory[currSection].size();

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
                const std::vector<uint8_t> instructionBytes = parseInstruction(firstToken, args);
                memory[currSection].insert(memory[currSection].end(), instructionBytes.begin(),
                                           instructionBytes.end());
                break;
            }
            case TokenType::LABEL: {
                pendingLabels.push_back(firstToken.value);
                break;
            }
            default: {
                throw std::runtime_error(
                        "Encountered unknown token type during parsing for token " +
                        firstToken.value);
            }
        }

        // Assign pending labels to current instruction or directive
        if (firstToken.type == TokenType::DIRECTIVE || firstToken.type == TokenType::INSTRUCTION) {
            for (const std::string& label : pendingLabels) {
                if (labelMap.contains(label))
                    throw std::runtime_error("Duplicate label " + label);
                labelMap[label] = memSectionOffset(currSection) + initialMemSecSize;
            }
            pendingLabels.clear();
        }
    }

    return memory;
}
