

#include <iostream>
#include <string>
#include <vector>

#include "CLI/CLI.hpp"
#include "fileio.h"
#include "interpreter.h"
#include "parser.h"


int main(const int argc, char* argv[]) {

    std::string inputFileName;

    CLI::App app{"masm - MIPS Interpreter", "masm"};
    app.add_option("input-file", inputFileName, "Input file to load")->required();

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        return app.exit(e);
    }

    try {
        const std::vector<std::string> lines = readFileLines(inputFileName);

        const std::vector<std::vector<Token>> program = Tokenizer::tokenize(lines);

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
