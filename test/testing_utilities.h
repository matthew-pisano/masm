//
// Created by matthew on 5/22/25.
//

#ifndef TESTING_UTILITIES_H
#define TESTING_UTILITIES_H
#include <vector>

#include "interpreter/interpreter.h"
#include "parser/parser.h"
#include "tokenizer/tokenizer.h"

class DebugParser : public Parser {

public:
    /**
     * Fetches the label map associated with this parser
     * @return The label map associated with this parser
     */
    LabelMap& getLabels();
};


/**
 * Converts a vector of integers to a vector of bytes
 * @param intVec The vector of integers to convert
 * @return The vector of bytes
 */
std::vector<std::byte> intVec2ByteVec(const std::vector<int>& intVec);


/**
 * Wraps a series of lines into a source file
 * @param lines The lines to wrap
 * @return A source file containing the lines
 */
SourceFile makeRawFile(const std::vector<std::string>& lines);


/**
 * Validates the token lines of a file against the expected tokens, fails if the tokens do not match
 * @param expectedTokens The expected tokens to compare against
 * @param actualTokens The actual tokens to compare
 * @throw runtime_error when the tokens do not match
 */
void validateTokenLines(const std::vector<std::vector<Token>>& expectedTokens,
                        const std::vector<LineTokens>& actualTokens);

#endif // TESTING_UTILITIES_H
