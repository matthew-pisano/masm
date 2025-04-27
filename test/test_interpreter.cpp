//
// Created by matthew on 4/26/25.
//


#include <catch2/catch_test_macros.hpp>
#include <string>
#include <vector>

#include "fileio.h"
#include "interpreter.h"
#include "parser.h"
#include "tokenizer.h"


/**
 * Validates the output of a program versus the expected output
 * @param sourceFileName The name of the source file to execute
 * @param logFileName The name of the output log to compare against
 */
void validateOutput(const std::string& sourceFileName, const std::string& logFileName) {
    const std::vector<std::string> logLines = readFileLines(logFileName);
    const std::vector<std::string> sourceLines = readFileLines(sourceFileName);

    Tokenizer tokenizer{};
    const std::vector<std::vector<Token>> program = tokenizer.tokenize(sourceLines);

    Parser parser{};
    const MemLayout layout = parser.parse(program);

    std::ostringstream oss;
    Interpreter interpreter{std::cin, oss};
    const int exitCode = interpreter.interpret(layout);

    REQUIRE(exitCode == 0);

    std::string actualLines = oss.str();
    for (const std::string& logLine : logLines) {
        std::string actualLine = actualLines.substr(0, actualLines.find('\n'));
        REQUIRE(actualLine == logLine);
        actualLines.erase(0, actualLine.length() + 1);
    }
}


TEST_CASE("Test Execute Hello World") {
    const std::string test_case = "hello_world";
    validateOutput("test/fixtures/" + test_case + "/" + test_case + ".asm",
                   "test/fixtures/" + test_case + "/" + test_case + ".txt");
}


TEST_CASE("Test Execute Arithmetic") {
    const std::string test_case = "arithmetic";
    validateOutput("test/fixtures/" + test_case + "/" + test_case + ".asm",
                   "test/fixtures/" + test_case + "/" + test_case + ".txt");
}


TEST_CASE("Test Execute Load Address") {
    const std::string test_case = "load_address";
    validateOutput("test/fixtures/" + test_case + "/" + test_case + ".asm",
                   "test/fixtures/" + test_case + "/" + test_case + ".txt");
}


TEST_CASE("Test Execute Loops") {
    const std::string test_case = "loops";
    validateOutput("test/fixtures/" + test_case + "/" + test_case + ".asm",
                   "test/fixtures/" + test_case + "/" + test_case + ".txt");
}


TEST_CASE("Test Execute Syscall") {
    const std::string test_case = "syscall";
    validateOutput("test/fixtures/" + test_case + "/" + test_case + ".asm",
                   "test/fixtures/" + test_case + "/" + test_case + ".txt");
}
