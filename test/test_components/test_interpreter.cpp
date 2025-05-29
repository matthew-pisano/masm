//
// Created by matthew on 4/26/25.
//


#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <string>
#include <vector>

#include "../testing_utilities.h"
#include "exceptions.h"
#include "interpreter/interpreter.h"
#include "io/fileio.h"
#include "parser/parser.h"
#include "tokenizer/tokenizer.h"
#include "utils.h"


/**
 * Validates the output of a program versus the expected output
 * @param ioMode The I/O mode to use for the interpreter
 * @param sourceFileNames The names of the source files to tokenize
 * @param logFileName The name of the output log to compare against
 * @param inputString The input string to provide to the program
 */
void validateOutput(const IOMode ioMode, const std::vector<std::string>& sourceFileNames,
                    const std::string& logFileName, const std::string& inputString = "") {
    const std::string logSource = readFile(logFileName);
    std::vector<std::string> logLines;
    std::stringstream ss(logSource);
    std::string line;
    while (std::getline(ss, line))
        logLines.push_back(line);

    std::vector<SourceFile> sourceFiles;
    sourceFiles.reserve(sourceFileNames.size()); // Preallocate memory for performance
    for (const std::string& fileName : sourceFileNames)
        sourceFiles.push_back({getFileBasename(fileName), readFile(fileName)});

    const std::vector<LineTokens> program = Tokenizer::tokenize(sourceFiles);

    Parser parser{};
    const MemLayout layout = parser.parse(program);

    int exitCode = 0;
    std::istringstream iss(inputString);
    std::ostringstream oss;

    DebugInterpreter interpreter{ioMode, iss, oss};
    exitCode = interpreter.interpret(layout);

    REQUIRE(exitCode == 0);

    std::string actualLines = oss.str();
    for (const std::string& logLine : logLines) {
        std::string actualLine = actualLines.substr(0, actualLines.find('\n'));
        REQUIRE(actualLine == logLine);
        actualLines.erase(0, actualLine.length() + 1);
    }

    REQUIRE(actualLines.empty());
}


TEST_CASE("Test Runtime Error") {
    std::vector<SourceFile> sourceFiles = {{"test.asm", "main:\n div $zero, $zero"}};
    const std::vector<LineTokens> program = Tokenizer::tokenize(sourceFiles);

    Parser parser{};
    const MemLayout layout = parser.parse(program);

    std::istringstream iss;
    std::ostringstream oss;

    DebugInterpreter interpreter{IOMode::SYSCALL, iss, oss};

    REQUIRE_THROWS_MATCHES(
            interpreter.interpret(layout), MasmRuntimeError,
            Catch::Matchers::Message(
                    "Runtime error at 0x00400000 (test.asm:2) -> Division by zero in "
                    "DIV instruction"));
}


TEST_CASE("Test Execute Hello World") {
    const std::string test_case = "hello_world";
    validateOutput(IOMode::SYSCALL, {"test/fixtures/" + test_case + "/" + test_case + ".asm"},
                   "test/fixtures/" + test_case + "/" + test_case + ".txt");
}


TEST_CASE("Test Execute Arithmetic") {
    const std::string test_case = "arithmetic";
    validateOutput(IOMode::SYSCALL, {"test/fixtures/" + test_case + "/" + test_case + ".asm"},
                   "test/fixtures/" + test_case + "/" + test_case + ".txt");
}


TEST_CASE("Test Execute Load Address") {
    const std::string test_case = "load_address";
    validateOutput(IOMode::SYSCALL, {"test/fixtures/" + test_case + "/" + test_case + ".asm"},
                   "test/fixtures/" + test_case + "/" + test_case + ".txt");
}


TEST_CASE("Test Execute Loops") {
    const std::string test_case = "loops";
    validateOutput(IOMode::SYSCALL, {"test/fixtures/" + test_case + "/" + test_case + ".asm"},
                   "test/fixtures/" + test_case + "/" + test_case + ".txt");
}


TEST_CASE("Test Execute Syscall") {
    const std::string test_case = "syscall";
    validateOutput(IOMode::SYSCALL, {"test/fixtures/" + test_case + "/" + test_case + ".asm"},
                   "test/fixtures/" + test_case + "/" + test_case + ".txt");
}

TEST_CASE("Test Execute Globals") {
    const std::string test_case = "globals";
    validateOutput(IOMode::SYSCALL,
                   {"test/fixtures/" + test_case + "/globalsOne.asm",
                    "test/fixtures/" + test_case + "/globalsTwo.asm"},
                   "test/fixtures/" + test_case + "/globalsOne.txt");
}


TEST_CASE("Test Execute Syscall Input Output") {
    const std::string test_case = "input_output";
    const std::string inputString = "5\n";
    validateOutput(IOMode::SYSCALL, {"test/fixtures/" + test_case + "/" + test_case + ".asm"},
                   "test/fixtures/" + test_case + "/" + test_case + ".txt", inputString);
}


TEST_CASE("Test Execute MMIO Input Output") {
    const std::string test_case = "mmio";
    const std::string inputString = "1234";
    validateOutput(IOMode::MMIO, {"test/fixtures/" + test_case + "/" + test_case + ".asm"},
                   "test/fixtures/" + test_case + "/" + test_case + ".txt", inputString);
}
