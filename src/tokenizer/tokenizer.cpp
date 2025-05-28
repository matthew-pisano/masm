//
// Created by matthew on 4/13/25.
//

#include "tokenizer/tokenizer.h"

#include <ostream>
#include <regex>
#include <string>
#include <vector>

#include "parser/instruction.h"
#include "tokenizer/postprocessor.h"
#include "exceptions.h"
#include "utils.h"


/**
 * An array storing the names of memory directives
 */
constexpr std::array<const char*, 14> tokenTypeNames = {
        "UNKNOWN",    "SEC_DIRECTIVE", "ALLOC_DIRECTIVE", "META_DIRECTIVE", "LABEL_DEF",
        "LABEL_REF",  "INSTRUCTION",   "REGISTER",        "IMMEDIATE",      "SEPERATOR",
        "OPEN_PAREN", "CLOSE_PAREN",   "STRING",          "MACRO_PARAM"};


std::string tokenTypeToString(TokenType t) { return tokenTypeNames.at(static_cast<int>(t)); }

bool operator==(const Token& lhs, const Token& rhs) {
    return lhs.type == rhs.type && lhs.value == rhs.value;
}

bool operator!=(const Token& lhs, const Token& rhs) {
    return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& os, const Token& t) {
    return os << "<" << tokenTypeToString(t.type) << ", \"" << t.value << "\">";
}


const std::array<std::string, 4> Tokenizer::secDirectives = {"data", "text", "kdata", "ktext"};
const std::array<std::string, 5> Tokenizer::metaDirectives = {"globl", "eqv", "macro", "end_macro",
                                                              "include"};


std::vector<SourceLine> Tokenizer::tokenize(const std::vector<RawFile>& rawFiles) {
    std::map<std::string, std::vector<SourceLine>> rawProgramMap;
    for (const auto& rawFile : rawFiles) {
        std::vector<SourceLine> fileTokens = tokenizeFile(rawFile);
        Postprocessor::processBaseAddressing(fileTokens);
        rawProgramMap[rawFile.name] = fileTokens;
    }

    // Process file inclusions
    Postprocessor::processIncludes(rawProgramMap);

    std::map<std::string, std::vector<SourceLine>> programMap;
    for (std::pair<const std::string, std::vector<SourceLine>>& fileTokens : rawProgramMap) {
        Postprocessor::replaceEqv(fileTokens.second);
        Postprocessor::processMacros(fileTokens.second);
        programMap["masm_mangle_file_" + fileTokens.first] = fileTokens.second;
    }

    // Mangle labels in files
    Postprocessor::mangleLabels(programMap);

    std::vector<SourceLine> program;
    for (std::pair<const std::string, std::vector<SourceLine>>& programFile : programMap)
        program.insert(program.end(), programFile.second.begin(), programFile.second.end());

    return program;
}


std::vector<SourceLine> Tokenizer::tokenizeFile(const RawFile& rawFile) {
    std::vector<SourceLine> tokenizedFile = {};

    for (size_t i = 0; i < rawFile.lines.size(); ++i) {
        const std::string& rawLine = rawFile.lines[i];
        std::vector<SourceLine> tokenizedLines;
        tokenizedLines = tokenizeLine(rawLine, i + 1);
        // Skip empty or comment lines
        if (tokenizedLines.empty())
            continue;

        for (SourceLine& line : tokenizedLines) {
            if (line.tokens.empty())
                continue;

            tokenizedFile.push_back(line);
        }
    }

    return tokenizedFile;
}


std::vector<SourceLine> Tokenizer::tokenizeLine(const std::string& rawLine, const size_t lineno) {
    const Token eqvToken = {TokenType::META_DIRECTIVE, "eqv"};

    std::vector<SourceLine> tokens = {{lineno, {}}};
    std::string currentToken;
    TokenType currentType = TokenType::UNKNOWN;
    char prevChar = '\0';

    // Append space to ensure all characters are tokenized
    for (const char c : rawLine + ' ') {
        SourceLine& tokenLine = tokens[tokens.size() - 1];
        // When in a string, gather all characters into a single token
        if (currentType == TokenType::STRING) {
            // Terminate the string when reaching unescaped quote
            if (c == '"' && prevChar != '\\') {
                currentType = TokenType::UNKNOWN;
                tokenLine.tokens.push_back({TokenType::STRING, currentToken});
                currentToken.clear();
            } else {
                // Add character to the string token
                currentToken += c;
            }

            prevChar = c;
        }
        // Begin a string at an open quote
        else if (c == '"') {
            currentType = TokenType::STRING;
            if (!currentToken.empty())
                throw MasmSyntaxError("Unexpected token '" + currentToken + "'", lineno);
        }
        // Skip remainder of line when reaching a comment
        else if (c == '#') {
            break;
        }
        // When reaching a natural token terminator
        else if (isspace(c) || c == ',' || c == ':' || c == '(' || c == ')') {
            terminateToken(c, currentType, currentToken, tokens);
        }
        // When the token type has already been decided
        else if (currentType != TokenType::UNKNOWN) {
            // Accumulate character into the current token
            currentToken += c;
        }
        // A preceding dot marks the token as a directive
        else if (c == '.') {
            currentType = TokenType::ALLOC_DIRECTIVE;
        }
        // A preceding dollar sign marks the token as a register
        else if (c == '$') {
            currentType = TokenType::REGISTER;
        }
        // A preceding percentage sign marks the token as a macro parameter
        else if (c == '%') {
            currentType = TokenType::MACRO_PARAM;
        }
        // Handle Immediates, Instructions, and Label references
        else if (isdigit(c) || c == '-' ||
                 (c == 'x' && currentToken.size() == 1 && currentToken[0] == '0')) {
            currentType = TokenType::IMMEDIATE;
            currentToken += c;
        } else {
            // Set first token in line as an instruction, otherwise as a label reference
            // If the third token in an eqv directive line, also set as instruction
            if (tokenLine.tokens.empty() ||
                (tokenLine.tokens.size() == 2 && tokenLine.tokens[0] == eqvToken))
                currentType = TokenType::INSTRUCTION;
            else
                currentType = TokenType::LABEL_REF;
            currentToken += c;
        }
    }

    if (!currentToken.empty())
        throw MasmSyntaxError("Unexpected EOL while parsing token '" + currentToken + "'", lineno);

    return tokens;
}


void Tokenizer::terminateToken(const char c, TokenType& currentType, std::string& currentToken,
                               std::vector<SourceLine>& tokens) {
    SourceLine& tokenLine = tokens[tokens.size() - 1];

    if (!isspace(c) && currentToken.empty() && tokenLine.tokens.empty())
        throw MasmSyntaxError("Unexpected token '" + std::string(1, c) + "'", tokenLine.lineno);

    if (currentType == TokenType::IMMEDIATE && currentToken.starts_with("0x"))
        currentToken = hexToInt(currentToken);

    // Assign the current token as a label definition if a colon follows
    if (c == ':')
        currentType = TokenType::LABEL_DEF;

    // Reassign directive as section directive if directive is data, text, etc.
    if (currentType == TokenType::ALLOC_DIRECTIVE &&
        std::ranges::find(secDirectives, currentToken) != secDirectives.end())
        currentType = TokenType::SEC_DIRECTIVE;
    // Reassign directive as meta directive if directive is .globl, .macro, etc.
    else if (currentType == TokenType::ALLOC_DIRECTIVE &&
             std::ranges::find(metaDirectives, currentToken) != metaDirectives.end())
        currentType = TokenType::META_DIRECTIVE;

    // Correct for labels put at beginning of line
    if (currentType == TokenType::INSTRUCTION && !isInstruction(currentToken))
        currentType = TokenType::LABEL_REF;

    // Add the current token to the vector and reset
    if (!currentToken.empty()) {
        tokenLine.tokens.push_back({currentType, currentToken});
        currentToken.clear();
        currentType = TokenType::UNKNOWN;
    }

    // Start a new line if a label was given
    if (c == ':')
        tokens.push_back({tokenLine.lineno, {}});
    else if (c == ',')
        tokenLine.tokens.push_back({TokenType::SEPERATOR, ","});
    else if (c == '(')
        tokenLine.tokens.push_back({TokenType::OPEN_PAREN, "("});
    else if (c == ')')
        tokenLine.tokens.push_back({TokenType::CLOSE_PAREN, ")"});
}
