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
 * Utility class that allows tests to access the internal state of an interpreter
 */
class DebugInterpreter : public Interpreter {

public:
    /**
     * Constructor for the DebugInterpreter class
     * @param ioMode The I/O mode to use for the interpreter
     * @param streamHandle The stream handle to use for I/O operations
     */
    DebugInterpreter(const IOMode ioMode, StreamHandle& streamHandle) :
        Interpreter(ioMode, streamHandle) {}

    /**
     * Constructor for the DebugInterpreter class
     * @param ioMode The I/O mode to use for the interpreter
     * @param streamHandle The stream handle to use for I/O operations
     * @param useLittleEndian Whether to use little-endian byte order for memory layout
     */
    DebugInterpreter(const IOMode ioMode, StreamHandle& streamHandle, const bool useLittleEndian) :
        Interpreter(ioMode, streamHandle, useLittleEndian) {}

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
