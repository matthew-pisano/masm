//
// Created by matthew on 4/13/25.
//

#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <array>
#include <map>
#include <string>
#include <vector>


/**
 * All valid types for tokens
 */
enum class TokenType {
    UNKNOWN,
    SEC_DIRECTIVE, // Directives that denote the state of a memory section
    ALLOC_DIRECTIVE, // Directives that allocate memory
    META_DIRECTIVE, // Directives that affect the structure of the program (globals, macros, etc.)
    LABEL_DEF,
    LABEL_REF,
    INSTRUCTION,
    REGISTER,
    IMMEDIATE,
    SEPERATOR,
    OPEN_PAREN,
    CLOSE_PAREN,
    STRING,
    MACRO_PARAM
};


/**
 * Returns a string representation of the given token type
 * @param t The token type to parse
 * @return The string representation
 */
std::string tokenTypeToString(TokenType t);


struct RawFile {
    std::string name;
    std::vector<std::string> lines;
};


/**
 * Class containing the type and text value of a token
 */
struct Token {
    TokenType type;
    std::string value;

    struct HashFunction {
        size_t operator()(const Token& token) const {
            const size_t typeHash = std::hash<int>()(static_cast<int>(token.type));
            const size_t valueHash = std::hash<std::string>()(token.value) << 1;
            return typeHash ^ valueHash;
        }
    };
};

bool operator==(const Token& lhs, const Token& rhs);
std::ostream& operator<<(std::ostream& os, const Token& t);


/**
 * Class to represent a line of source code
 */
struct SourceLine {
    size_t lineno;
    std::vector<Token> tokens;
};


/**
 * Class to tokenize incoming source code lines into parsable tokens
 */
class Tokenizer {

    const static std::array<std::string, 2> secDirectives;
    const static std::array<std::string, 4> metaDirectives;

    /**
     * A helper function that terminates the current token and starts a new one
     * @param c The character that terminated the token
     * @param currentType The type of the current token
     * @param currentToken The current token to terminate
     * @param tokens The vector of source code lines to add the current token to
     */
    static void terminateToken(char c, TokenType& currentType, std::string& currentToken,
                               std::vector<SourceLine>& tokens);

    /**
     * A helper function that tokenizes single lines.  Multiple token lines may be produced
     * @param rawLine The line of source code to tokenize
     * @param lineno The line number of the source code
     * @return A vector of source code lines
     * @throw MasmSyntaxError When encountering a malformed or early terminating line
     */
    static std::vector<SourceLine> tokenizeLine(const std::string& rawLine, size_t lineno);

public:
    /**
     * Tokenizes incoming source code lines into parsable tokens
     * @param rawFile The lines of source code to tokenize
     * @return A vector of source code lines
     * @throw MasmSyntaxError When encountering a malformed or early terminating file
     */
    static std::vector<SourceLine> tokenizeFile(const RawFile& rawFile);

    /**
     * Tokenizes incoming source code lines from multiple files into parsable tokens
     * @param rawFiles The lines of source code to tokenize
     * @return A vector of source code lines
     * @throw MasmSyntaxError When encountering a malformed or early terminating file
     */
    [[nodiscard]] static std::vector<SourceLine> tokenize(const std::vector<RawFile>& rawFiles);
};

#endif // TOKENIZER_H
