//
// Created by matthew on 4/13/25.
//

#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <string>
#include <vector>


/**
 * All valid categories for tokens
 */
enum class TokenCategory {
    UNKNOWN,
    SEC_DIRECTIVE, // Directives that denote the state of a memory section
    ALLOC_DIRECTIVE, // Directives that allocate memory
    META_DIRECTIVE, // Directives that affect the structure of the program (globals, macros, etc.)
    LABEL_DEF,
    LABEL_REF,
    INSTRUCTION,
    REGISTER,
    IMMEDIATE,
    SEPERATOR, // Used to separate tokens, such as commas
    OPEN_PAREN,
    CLOSE_PAREN,
    STRING,
    MACRO_PARAM
};


/**
 * Returns a string representation (name) of the given token type
 * @param t The token type to parse
 * @return The string representation
 */
std::string TokenCategoryToString(TokenCategory t);


/**
 * Class to represent a raw file containing source code
 */
struct SourceFile {
    /**
     * The name of the file, used for error reporting and debugging
     */
    std::string name;

    /**
     * The source code of the file
     */
    std::string source;
};


/**
 * Class containing the type and text value of a token
 */
struct Token {
    /**
     * The type of the token, used to determine how to parse it
     */
    TokenCategory type;

    /**
     * The text value of the token, which is the raw string representation
     */
    std::string value;

    /**
     * Constructs a token with the given type and value (used for mappings with tokens as keys)
     */
    struct HashFunction {
        size_t operator()(const Token& token) const {
            const size_t typeHash = std::hash<int>()(static_cast<int>(token.type));
            const size_t valueHash = std::hash<std::string>()(token.value) << 1;
            return typeHash ^ valueHash;
        }
    };
};

bool operator==(const Token& lhs, const Token& rhs);
bool operator!=(const Token& lhs, const Token& rhs);
std::ostream& operator<<(std::ostream& os, const Token& t);


/**
 * Class to represent a tokenized line of source code
 */
struct LineTokens {
    /**
     * The name of the source file this line belongs to, used for error reporting
     */
    std::string filename;

    /**
     * The line number of the source code, used for error reporting
     */
    size_t lineno;

    /**
     * The tokens in this line, which are the parsed tokens from the source code
     */
    std::vector<Token> tokens;
};

bool operator==(const LineTokens& lhs, const LineTokens& rhs);
bool operator!=(const LineTokens& lhs, const LineTokens& rhs);


/**
 * Class to tokenize incoming source code lines into parsable tokens
 */
class Tokenizer {

    /**
     * A helper function that terminates the current token and starts a new one
     * @param c The character that terminated the token
     * @param currentType The type of the current token
     * @param currentToken The current token to terminate
     * @param tokens The vector of source code lines to add the current token to
     */
    static void terminateToken(char c, TokenCategory& currentType, std::string& currentToken,
                               std::vector<LineTokens>& tokens);

    /**
     * A helper function that tokenizes single lines.  Multiple token lines may be produced
     * @param sourceLine The line of source code to tokenize
     * @param filename The name of the source file
     * @param lineno The line number of the source code
     * @return A vector of source code lines
     * @throw MasmSyntaxError When encountering a malformed or early terminating line
     */
    static std::vector<LineTokens> tokenizeLine(const std::string& sourceLine,
                                                const std::string& filename, size_t lineno);

public:
    /**
     * Tokenizes incoming source code lines into parsable tokens
     * @param sourceFile The lines of source code to tokenize
     * @return A vector of source code lines
     * @throw MasmSyntaxError When encountering a malformed or early terminating file
     */
    static std::vector<LineTokens> tokenizeFile(const SourceFile& sourceFile);

    /**
     * Tokenizes and post-processes source code lines from multiple files into parsable tokens
     * @param sourceFiles The lines of source code to tokenize
     * @return A vector of source code lines
     * @throw MasmSyntaxError When encountering a malformed or early terminating file
     */
    [[nodiscard]] static std::vector<LineTokens>
    tokenize(const std::vector<SourceFile>& sourceFiles);
};

#endif // TOKENIZER_H
