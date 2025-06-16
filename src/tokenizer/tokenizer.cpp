//
// Created by matthew on 4/13/25.
//

#include "tokenizer/tokenizer.h"

#include <ostream>
#include <regex>
#include <string>
#include <vector>

#include "exceptions.h"
#include "parser/directive.h"
#include "parser/instruction.h"
#include "tokenizer/postprocessor.h"
#include "utils.h"


/**
 * An array storing the names of memory directives
 */
constexpr std::array<const char*, 14> tokenCategoryNames = {
        "UNKNOWN",    "SEC_DIRECTIVE", "ALLOC_DIRECTIVE", "META_DIRECTIVE", "LABEL_DEF",
        "LABEL_REF",  "INSTRUCTION",   "REGISTER",        "IMMEDIATE",      "SEPERATOR",
        "OPEN_PAREN", "CLOSE_PAREN",   "STRING",          "MACRO_PARAM"};


bool operator==(const LineTokens& lhs, const LineTokens& rhs) {
    return lhs.filename == rhs.filename && lhs.lineno == rhs.lineno;
}
bool operator!=(const LineTokens& lhs, const LineTokens& rhs) { return !(lhs == rhs); }


std::string tokenCategoryToString(TokenCategory category) {
    return tokenCategoryNames.at(static_cast<int>(category));
}

bool operator==(const Token& lhs, const Token& rhs) {
    return lhs.category == rhs.category && lhs.value == rhs.value;
}

bool operator!=(const Token& lhs, const Token& rhs) { return !(lhs == rhs); }

std::ostream& operator<<(std::ostream& os, const Token& t) {
    return os << "<" << tokenCategoryToString(t.category) << ", \"" << t.value << "\">";
}


std::vector<LineTokens> Tokenizer::tokenize(const std::vector<SourceFile>& sourceFiles) {
    std::map<std::string, std::vector<LineTokens>> rawProgramMap;

    // Tokenize each source file and process base addressing
    for (const auto& rawFile : sourceFiles) {
        std::vector<LineTokens> fileTokens = tokenizeFile(rawFile);
        Postprocessor::processBaseAddressing(fileTokens);
        rawProgramMap[rawFile.name] = fileTokens;
    }

    // Process file inclusions
    Postprocessor::processIncludes(rawProgramMap);

    std::map<std::string, std::vector<LineTokens>> programMap;

    // Process macros and eqv directives in each file
    for (std::pair<const std::string, std::vector<LineTokens>>& fileTokens : rawProgramMap) {
        Postprocessor::replaceEqv(fileTokens.second);
        Postprocessor::processMacros(fileTokens.second);
        programMap[fileTokens.first] = fileTokens.second;
    }

    // Mangle labels in files
    Postprocessor::mangleLabels(programMap);

    // Combine all tokenized lines into a single program vector
    std::vector<LineTokens> program;
    for (const SourceFile& sourceFile : sourceFiles) {
        std::vector<LineTokens> fileLines = programMap[sourceFile.name];
        program.insert(program.end(), fileLines.begin(), fileLines.end());
    }

    return program;
}


std::vector<LineTokens> Tokenizer::tokenizeFile(const SourceFile& sourceFile) {
    std::vector<LineTokens> tokenizedFile = {};

    // Split the source code into lines
    std::stringstream ss(sourceFile.source);
    std::string line;
    std::vector<std::string> sourceLines;
    while (std::getline(ss, line))
        sourceLines.push_back(line);

    for (size_t i = 0; i < sourceLines.size(); ++i) {
        std::vector<LineTokens> tokenizedLines =
                tokenizeLine(sourceLines[i], sourceFile.name, i + 1);

        for (LineTokens& tokenLine : tokenizedLines)
            // Skip empty or comment lines
            if (!tokenLine.tokens.empty())
                tokenizedFile.push_back(tokenLine);
    }

    return tokenizedFile;
}


std::vector<LineTokens> Tokenizer::tokenizeLine(const std::string& sourceLine,
                                                const std::string& filename, const size_t lineno) {
    const Token eqvToken = {TokenCategory::META_DIRECTIVE, "eqv"};

    std::vector<LineTokens> tokens = {{filename, lineno, {}}};
    std::string currentToken;
    TokenCategory currentType = TokenCategory::UNKNOWN;
    char prevChar = '\0';

    // Append space to ensure all characters are tokenized
    for (const char c : sourceLine + ' ') {
        LineTokens& tokenLine = tokens[tokens.size() - 1];
        // When in a string, gather all characters into a single token
        if (currentType == TokenCategory::STRING) {
            // Terminate the string when reaching unescaped quote
            if (c == '"' && prevChar != '\\') {
                currentType = TokenCategory::UNKNOWN;
                tokenLine.tokens.push_back({TokenCategory::STRING, currentToken});
                currentToken.clear();
            } else {
                // Add character to the string token
                currentToken += c;
            }

            prevChar = c;
        }
        // Begin a string at an open quote
        else if (c == '"') {
            currentType = TokenCategory::STRING;
            if (!currentToken.empty())
                throw MasmSyntaxError("Unexpected token '" + currentToken + "'", filename, lineno);
        }
        // Skip remainder of line when reaching a comment
        else if (c == '#') {
            break;
        }
        // When reaching a natural token terminator
        else if (isspace(c) || c == ',' || c == ':' || c == '(' || c == ')') {
            terminateToken(c, currentType, currentToken, tokens);
        }
        // When the token category has already been decided
        else if (currentType != TokenCategory::UNKNOWN) {
            // Accumulate character into the current token
            currentToken += c;
        }
        // A preceding dot marks the token as a directive
        else if (c == '.') {
            currentType = TokenCategory::ALLOC_DIRECTIVE;
        }
        // A preceding dollar sign marks the token as a register
        else if (c == '$') {
            currentType = TokenCategory::REGISTER;
        }
        // A preceding percentage sign marks the token as a macro parameter
        else if (c == '%') {
            currentType = TokenCategory::MACRO_PARAM;
        }
        // Handle Immediates, Instructions, and Label references
        else if (isdigit(c) || c == '-' ||
                 (c == 'x' && currentToken.size() == 1 && currentToken[0] == '0')) {
            currentType = TokenCategory::IMMEDIATE;
            currentToken += c;
        } else {
            // Set first token in line as an instruction, otherwise as a label reference
            // If the third token in an eqv directive line, also set as instruction
            if (tokenLine.tokens.empty() ||
                (tokenLine.tokens.size() == 2 && tokenLine.tokens[0] == eqvToken))
                currentType = TokenCategory::INSTRUCTION;
            else
                currentType = TokenCategory::LABEL_REF;
            currentToken += c;
        }
    }

    // If the current token is not terminated before the end of the file (usually with strings)
    if (!currentToken.empty())
        throw MasmSyntaxError("Unexpected EOL while parsing token '" + currentToken + "'", filename,
                              lineno);

    return tokens;
}


void Tokenizer::terminateToken(const char c, TokenCategory& currentType, std::string& currentToken,
                               std::vector<LineTokens>& tokens) {
    LineTokens& tokenLine = tokens[tokens.size() - 1];

    if (!isspace(c) && currentToken.empty() && tokenLine.tokens.empty())
        throw MasmSyntaxError("Unexpected token '" + std::string(1, c) + "'", tokenLine.filename,
                              tokenLine.lineno);

    if (currentType == TokenCategory::IMMEDIATE && currentToken.starts_with("0x"))
        currentToken = hexToInt(currentToken);

    // Assign the current token as a label definition if a colon follows
    if (c == ':')
        currentType = TokenCategory::LABEL_DEF;

    // Reassign directive as section directive if directive is data, text, etc.
    if (currentType == TokenCategory::ALLOC_DIRECTIVE &&
        std::ranges::find(MEM_SEC_DIRECTIVES, currentToken) != MEM_SEC_DIRECTIVES.end())
        currentType = TokenCategory::SEC_DIRECTIVE;
    // Reassign directive as meta directive if directive is .globl, .macro, etc.
    else if (currentType == TokenCategory::ALLOC_DIRECTIVE &&
             std::ranges::find(META_DIRECTIVES, currentToken) != META_DIRECTIVES.end())
        currentType = TokenCategory::META_DIRECTIVE;

    // Correct for labels put at beginning of line
    if (currentType == TokenCategory::INSTRUCTION && !isInstruction(currentToken))
        currentType = TokenCategory::LABEL_REF;

    // Add the current token to the vector and reset
    if (!currentToken.empty()) {
        tokenLine.tokens.push_back({currentType, currentToken});
        currentToken.clear();
        currentType = TokenCategory::UNKNOWN;
    }

    // Start a new line if a label was given
    if (c == ':')
        tokens.push_back({tokenLine.filename, tokenLine.lineno, {}});
    else if (c == ',')
        tokenLine.tokens.push_back({TokenCategory::SEPERATOR, ","});
    else if (c == '(')
        tokenLine.tokens.push_back({TokenCategory::OPEN_PAREN, "("});
    else if (c == ')')
        tokenLine.tokens.push_back({TokenCategory::CLOSE_PAREN, ")"});
}
