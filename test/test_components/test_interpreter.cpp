//
// Created by matthew on 4/26/25.
//


#include <catch2/catch_test_macros.hpp>
#include <string>
#include <vector>

#include "interpreter/interpreter.h"
#include "io/fileio.h"
#include "parser/parser.h"
#include "tokenizer/tokenizer.h"
#include "../testing_utilities.h"
#include "utils.h"


/**
 * Validates the output of a program versus the expected output
 * @param sourceFileNames The names of the source files to tokenize
 * @param logFileName The name of the output log to compare against
 */
void validateOutput(const std::vector<std::string>& sourceFileNames,
                    const std::string& logFileName) {
    const std::vector<std::string> logLines = readFileLines(logFileName);

    std::vector<RawFile> sourceLines;
    sourceLines.reserve(sourceFileNames.size()); // Preallocate memory for performance
    for (const std::string& fileName : sourceFileNames)
        sourceLines.push_back({getFileBasename(fileName), readFileLines(fileName)});

    const std::vector<SourceLine> program = Tokenizer::tokenize(sourceLines);

    Parser parser{};
    const MemLayout layout = parser.parse(program);

    int exitCode = 0;
    std::ostringstream oss;

    DebugInterpreter interpreter{std::cin, oss};
    interpreter.setUpdateMMIO(false);
    exitCode = interpreter.interpret(layout);

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
    validateOutput({"test/fixtures/" + test_case + "/" + test_case + ".asm"},
                   "test/fixtures/" + test_case + "/" + test_case + ".txt");
}


TEST_CASE("Test Execute Arithmetic") {
    const std::string test_case = "arithmetic";
    validateOutput({"test/fixtures/" + test_case + "/" + test_case + ".asm"},
                   "test/fixtures/" + test_case + "/" + test_case + ".txt");
}


TEST_CASE("Test Execute Load Address") {
    const std::string test_case = "load_address";
    validateOutput({"test/fixtures/" + test_case + "/" + test_case + ".asm"},
                   "test/fixtures/" + test_case + "/" + test_case + ".txt");
}


TEST_CASE("Test Execute Loops") {
    const std::string test_case = "loops";
    validateOutput({"test/fixtures/" + test_case + "/" + test_case + ".asm"},
                   "test/fixtures/" + test_case + "/" + test_case + ".txt");
}


TEST_CASE("Test Execute Syscall") {
    const std::string test_case = "syscall";
    validateOutput({"test/fixtures/" + test_case + "/" + test_case + ".asm"},
                   "test/fixtures/" + test_case + "/" + test_case + ".txt");
}

TEST_CASE("Test Execute Globals") {
    const std::string test_case = "globals";
    validateOutput({"test/fixtures/" + test_case + "/globalsOne.asm",
                    "test/fixtures/" + test_case + "/globalsTwo.asm"},
                   "test/fixtures/" + test_case + "/globalsOne.txt");
}
