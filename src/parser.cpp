//
// Created by matthew on 4/14/25.
//

#include "parser.h"

#include <register.h>
#include <stdexcept>

#include "instruction.h"
#include "utils.h"


MemSection nameToMemSection(const std::string& name) {
    switch (name) {
        case "text":
            return MemSection::TEXT;
        case "data":
            return MemSection::DATA;
        default:
            throw std::runtime_error("Unknown memory directive " + name);
    }
}


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


std::vector<uint8_t> Parser::parseInstruction(const std::vector<Token>& instrTokens) {
    uint8_t machineCode{};
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
                break;
        }
    }

    return {machineCode};
}


MemLayout Parser::parse(const std::vector<std::vector<Token>>& tokens) {
    std::map<std::string, size_t> labelMap = {};
    MemSection currSection = MemSection::TEXT;
    MemLayout memory = {{currSection, {}}};

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
            case TokenType::INSTRUCTION:
                const std::vector<uint8_t> instructionBytes = parseInstruction(line);
                memory[currSection].insert(memory[currSection].end(), instructionBytes.begin(),
                                           instructionBytes.end());
            case TokenType::UNKNOWN:
                throw std::runtime_error(
                        "Encountered unknown token type during parsing for token " +
                        firstToken.value);
        }
    }

    return memory;
}
