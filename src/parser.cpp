//
// Created by matthew on 4/14/25.
//

#include "parser.h"

#include <stdexcept>

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


std::vector<Word> stringToWords(std::string string) {}


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


std::vector<Word> Parser::parseDirective(const std::vector<Token>& dirTokens) {

    std::vector<Word> words = {};

    switch (const std::string dirName = dirTokens[0].value) {
        case "asciiz":
            if (dirTokens.size() != 2)
                throw std::runtime_error(".asciiz expects exactly one argument");
            return stringToWords(dirTokens[1].value);
        case "word":
            if (dirTokens.size() < 2)
                throw std::runtime_error(".word expects at least one argument");

            const std::vector unfilteredArgs(dirTokens.begin() + 1, dirTokens.end());
            std::vector<Token> args = filterList(unfilteredArgs);

            for (const Token& arg : args) {
                if (arg.type != TokenType::IMMEDIATE || !isSignedInteger(arg.value))
                    throw std::runtime_error(".word expects integer values as arguments");
                words.push_back({static_cast<uint32_t>(std::stoi(arg.value)), std::nullopt});
            }
            return words;
        default:
            throw std::runtime_error("Unsupported directive " + dirName);
    }
}


MemLayout Parser::parse(std::vector<std::vector<Token>> tokens) {
    MemSection currSection = MemSection::TEXT;
    MemLayout memory = {{currSection, {}}};

    for (const std::vector<Token>& line : tokens) {
        if (line.empty())
            continue;

        const Token firstToken = line[0];
        switch (firstToken.type) {
            case TokenType::MEMDIRECTIVE:
                currSection = nameToMemSection(firstToken.value);
                if (!memory.contains(currSection))
                    memory[currSection] = {};
                break;
            case TokenType::DIRECTIVE:
                std::vector<Word> directiveWords = parseDirective(line);
                memory[currSection].insert(memory[currSection].end(), directiveWords.begin(),
                                           directiveWords.end());
            case TokenType::INSTRUCTION:
                const Word instructionWord = parseInstruction(line);
                memory[currSection].push_back(instructionWord);
            case TokenType::UNKNOWN:
                throw std::runtime_error(
                        "Encountered unknown token type during parsing for token " +
                        firstToken.value);
        }
    }

    return memory;
}
