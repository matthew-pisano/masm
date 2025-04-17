//
// Created by matthew on 4/14/25.
//

#include "parser.h"

#include <register.h>
#include <stdexcept>

#include "instruction.h"
#include "utils.h"


std::vector<uint8_t> stringToBytes(const std::string& string) {
    std::vector<uint8_t> bytes = {};
    for (const char c : string)
        bytes.push_back(static_cast<uint8_t>(c));
    return bytes;
}


std::vector<uint8_t> intStringToBytes(const std::string& string) {
    if (!isSignedInteger(string))
        throw std::runtime_error("Invalid integer " + string);

    const int integer = std::stoi(string);

    std::vector<uint8_t> bytes = {};
    // Using big endian
    bytes.push_back(static_cast<uint8_t>(integer >> 24 & 0xFF));
    bytes.push_back(static_cast<uint8_t>(integer >> 16 & 0xFF));
    bytes.push_back(static_cast<uint8_t>(integer >> 8 & 0xFF));
    bytes.push_back(static_cast<uint8_t>(integer & 0xFF));
    return bytes;
}


std::vector<Token> filterList(const std::vector<Token>& listTokens) {
    std::vector<Token> elements = {};

    for (size_t i = 0; i < listTokens.size(); i++) {
        if (i % 2 == 1 && listTokens[i].type != TokenType::SEPERATOR)
            throw std::runtime_error("Expected , after token " + listTokens[i - 1].value);
        if (i % 2 == 0 && listTokens[i].type == TokenType::SEPERATOR)
            throw std::runtime_error("Unexpected ,");
        if (i == listTokens.size() - 1 && listTokens[i].type == TokenType::SEPERATOR)
            throw std::runtime_error("Unexpected , after token " + listTokens[i - 1].value);

        if (listTokens[i].type == TokenType::SEPERATOR)
            continue;

        elements.push_back(listTokens[i]);
    }

    return elements;
}


bool tokenTypeMatch(const std::vector<TokenType>& pattern, const std::vector<Token>& tokens) {
    if (pattern.size() != tokens.size())
        return false;

    for (size_t i = 0; i < pattern.size(); i++)
        if (tokens[i].type != pattern[i])
            return false;

    return true;
}


std::vector<uint8_t> Parser::parseDirective(const std::vector<Token>& dirTokens) {

    std::vector<uint8_t> bytes = {};

    switch (const std::string dirName = dirTokens[0].value) {
        case "asciiz":
            if (dirTokens.size() != 2)
                throw std::runtime_error(".asciiz expects exactly one argument");
            return stringToBytes(dirTokens[1].value);
        case "word":
            if (dirTokens.size() < 2)
                throw std::runtime_error(".word expects at least one argument");

            const std::vector unfilteredArgs(dirTokens.begin() + 1, dirTokens.end());
            std::vector<Token> args = filterList(unfilteredArgs);

            for (const Token& arg : args) {
                if (arg.type != TokenType::IMMEDIATE)
                    throw std::runtime_error(".word expects immediate values as arguments");
                std::vector<uint8_t> immediateBytes = intStringToBytes(arg.value);
                bytes.insert(bytes.end(), immediateBytes.begin(), immediateBytes.end());
            }
            return bytes;
        default:
            throw std::runtime_error("Unsupported directive " + dirName);
    }
}


std::vector<uint8_t> Parser::parseRTypeInstruction(const uint32_t opcode, const uint32_t rs,
                                                   const uint32_t rt, const uint32_t rd,
                                                   const uint32_t shamt, const uint32_t funct) {

    // Combine fields into 32-bit instruction code
    const uint32_t instruction = (opcode & 0x3F) << 26 | (rs & 0x1F) << 21 | (rt & 0x1F) << 16 |
                                 (rd & 0x1F) << 11 | (shamt & 0x1F) << 6 | funct & 0x3F;

    // Break the instruction into 4 bytes (big-endian)
    std::vector<uint8_t> bytes(4);
    bytes[0] = (instruction >> 24) & 0xFF; // Most significant byte
    bytes[1] = (instruction >> 16) & 0xFF;
    bytes[2] = (instruction >> 8) & 0xFF;
    bytes[3] = instruction & 0xFF; // Least significant byte

    return bytes;
}


std::vector<uint8_t> Parser::parseITypeInstruction(const uint32_t opcode, const uint32_t rs,
                                                   const uint32_t rt, const uint32_t immediate) {

    // Combine fields into 32-bit instruction code
    const uint32_t instruction =
            (opcode & 0x3F) << 26 | (rs & 0x1F) << 21 | (rt & 0x1F) << 16 | (immediate & 0xFFFF);

    // Break the instruction into 4 bytes (big-endian)
    std::vector<uint8_t> bytes(4);
    bytes[0] = (instruction >> 24) & 0xFF; // Most significant byte
    bytes[1] = (instruction >> 16) & 0xFF;
    bytes[2] = (instruction >> 8) & 0xFF;
    bytes[3] = instruction & 0xFF; // Least significant byte

    return bytes;
}

std::vector<uint8_t> Parser::parseJTypeInstruction(const uint32_t opcode, const uint32_t address) {

    // Combine fields into 32-bit instruction code
    const uint32_t instruction = (opcode & 0x3F) << 26 | (address & 0x3FFFFFF);

    // Break the instruction into 4 bytes (big-endian)
    std::vector<uint8_t> bytes(4);
    bytes[0] = (instruction >> 24) & 0xFF; // Most significant byte
    bytes[1] = (instruction >> 16) & 0xFF;
    bytes[2] = (instruction >> 8) & 0xFF;
    bytes[3] = instruction & 0xFF; // Least significant byte

    return bytes;
}

std::vector<uint8_t> Parser::parseInstruction(const std::vector<Token>& instrTokens) {
    RegisterFile regFile{};

    const std::vector unfilteredArgs(instrTokens.begin() + 1, instrTokens.end());
    const std::vector<Token> args = filterList(unfilteredArgs);
    validateInstruction(instrTokens[0], args);

    InstructionOp instructionOp = nameToInstructionOp(instrTokens[0].value);
    std::vector<uint32_t> argCode = {};
    for (const Token& arg : args) {
        switch (arg.type) {
            case TokenType::IMMEDIATE:
                if (!isSignedInteger(arg.value))
                    throw std::runtime_error("Invalid integer " + arg.value);
                argCode.push_back(static_cast<uint32_t>(std::stoi(arg.value)));
                break;
            case TokenType::REGISTER:
                if (isSignedInteger(arg.value) && std::stoi(arg.value) >= 0)
                    argCode.push_back(std::stoi(arg.value));
                else
                    regFile.indexFromName(arg.value);
                break;
            case TokenType::LABELREF:
                if (!labelMap.contains(arg.value))
                    throw std::runtime_error("Unknown label " + arg.value);
                argCode.push_back(labelMap[arg.value]);
                break;
            default:
                throw std::runtime_error("Invalid argument type " +
                                         std::to_string(static_cast<int>(arg.type)));
        }
    }

    switch (instructionOp.type) {
        case InstructionType::R_TYPE:
            return parseRTypeInstruction(0, argCode[0], argCode[1], argCode[2], 0,
                                         instructionOp.opFuncCode);
        case InstructionType::I_TYPE:
            return parseITypeInstruction(instructionOp.opFuncCode, argCode[0], argCode[1],
                                         argCode[2]);
        case InstructionType::J_TYPE:
            return parseJTypeInstruction(instructionOp.opFuncCode, argCode[0]);
    }

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
        switch (firstToken.type) {
            case TokenType::MEMDIRECTIVE:
                currSection = nameToMemSection(firstToken.value);
                if (!memory.contains(currSection))
                    memory[currSection] = {};
                break;
            case TokenType::DIRECTIVE:
                std::vector<uint8_t> directiveBytes = parseDirective(line);
                memory[currSection].insert(memory[currSection].end(), directiveBytes.begin(),
                                           directiveBytes.end());
                break;
            case TokenType::INSTRUCTION:
                const std::vector<uint8_t> instructionBytes = parseInstruction(line);
                memory[currSection].insert(memory[currSection].end(), instructionBytes.begin(),
                                           instructionBytes.end());
                break;
            case TokenType::LABEL:
                pendingLabels.push_back(firstToken.value);
                break;
            case TokenType::UNKNOWN:
                throw std::runtime_error(
                        "Encountered unknown token type during parsing for token " +
                        firstToken.value);
        }

        if (firstToken.type == TokenType::DIRECTIVE && firstToken.type == TokenType::INSTRUCTION) {
            for (const std::string& label : pendingLabels) {
                if (labelMap.contains(label))
                    throw std::runtime_error("Duplicate label " + label);
                labelMap[label] =
                        memSectionOffset(currSection) + 8 * (memory[currSection].size() - 1);
            }
        }
    }

    return memory;
}
