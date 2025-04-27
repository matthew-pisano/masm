//
// Created by matthew on 4/13/25.
//

#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <map>
#include <string>
#include <vector>


/**
 * All valid types for tokens
 */
enum class TokenType {
    UNKNOWN,
    MEMDIRECTIVE,
    DIRECTIVE,
    LABEL,
    LABELREF,
    INSTRUCTION,
    REGISTER,
    IMMEDIATE,
    SEPERATOR,
    OPEN_PAREN,
    CLOSE_PAREN,
    STRING,
};


/**
 * Returns a string representation of the given token type
 * @param t The token type to parse
 * @return The string representation
 */
std::string tokenTypeToString(TokenType t);


/**
 * Class containing the type and text value of a token
 */
struct Token {
    TokenType type;
    std::string value;
};

bool operator==(const Token& lhs, const Token& rhs);
std::ostream& operator<<(std::ostream& os, const Token& t);


/**
 * Class to tokenize incoming source code lines into parsable tokens
 */
class Tokenizer {

    /**
     * All global labels within the file
     */
    std::vector<std::string> globals;

    /**
     * Modifies the give token line to replace the pattern of y($xx) with $xx, y to match MIPS
     * addressing mode when a close paren is reached
     * @param tokenLine The line of tokens to modify
     */
    static void processCloseParen(std::vector<Token>& tokenLine);

    /**
     * A helper function that mangles labels in the given line of tokens
     * @param availableLabels The list of available labels to mangle
     * @param lineTokens The line of tokens to mangle
     * @param fileId The file ID to append to the label
     */
    void mangleLabelsInLine(std::vector<std::string>& availableLabels,
                            std::vector<Token>& lineTokens, const std::string& fileId);

    /**
     * A helper function that tokenizes single lines.  Multiple token lines may be produced
     * @param rawLine The line of source code to tokenize
     * @return A vector of vectors of tokens, where each vector represents a tokenized line
     * @throw runtime_error When encountering a malformed or early terminating line
     */
    std::vector<std::vector<Token>> tokenizeLine(const std::string& rawLine);

public:
    /**
     * Tokenizes incoming source code lines into parsable tokens
     * @param rawLines The lines of source code to tokenize
     * @return A vector of vectors of tokens, where each vector represents a tokenized line
     * @throw runtime_error When encountering a malformed or early terminating file
     */
    [[nodiscard]] std::vector<std::vector<Token>>
    tokenize(const std::vector<std::string>& rawLines);

    /**
     * Name mangels tokens in the given program map by adding the file ID to the label
     * @param programMap The map of file IDs to their tokenized lines
     * @throw runtime_error When the file ID is empty
     */
    void mangleLabels(std::map<std::string, std::vector<std::vector<Token>>>& programMap);
};

#endif // TOKENIZER_H
