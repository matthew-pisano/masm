

#include <iostream>
#include <regex>
#include <string>
#include <vector>

#include "CLI/CLI.hpp"
#include "fileio.h"
#include "interpreter.h"
#include "parser.h"


int main(const int argc, char* argv[]) {

    std::vector<std::string> inputFileNames;

    CLI::App app{"masm - MIPS Interpreter", "masm"};
    app.add_option("input-file", inputFileNames, "Input file to load")
            ->required()
            ->allow_extra_args();

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        return app.exit(e);
    }

    try {
        std::vector<std::vector<std::string>> programLines;
        for (const std::string& fileName : inputFileNames)
            programLines.push_back(readFileLines(fileName));

        Tokenizer tokenizer{};
        const std::vector<std::vector<Token>> program = tokenizer.tokenize(programLines);

        Parser parser{};
        const MemLayout layout = parser.parse(program);

        Interpreter interpreter{};
        const int exitCode = interpreter.interpret(layout);
        return exitCode;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
