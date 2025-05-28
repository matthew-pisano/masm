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


class DebugInterpreter : public Interpreter {

public:
    explicit DebugInterpreter(const IOMode ioMode) : Interpreter(ioMode) {}
    DebugInterpreter(const IOMode ioMode, std::istream& input, std::ostream& output) :
        Interpreter(ioMode, input, output) {}

    /**
     * Gets the current state of the interpreter
     * @return The current state of the interpreter
     */
    State& getState();
};


/**
 * Converts a vector of integers to a vector of bytes
 * @param intVec The vector of integers to convert
 * @return The vector of bytes
 */
std::vector<std::byte> intVec2ByteVec(const std::vector<int>& intVec);


/**
 * Wraps a series of lines into a raw files
 * @param lines The lines to wrap
 * @return A raw file containing the lines
 */
RawFile makeRawFile(const std::vector<std::string>& lines);


/**
 * Validates the token lines of a file against the expected tokens, fails if the tokens do not match
 * @param expectedTokens The expected tokens to compare against
 * @param actualTokens The actual tokens to compare
 * @throw runtime_error when the tokens do not match
 */
void validateTokenLines(const std::vector<std::vector<Token>>& expectedTokens,
                        const std::vector<SourceLine>& actualTokens);

#endif // TESTING_UTILITIES_H
