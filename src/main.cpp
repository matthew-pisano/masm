#include <iostream>
#include <string>
#include <vector>

#include "interpreter/interpreter.h"
#include "io/consoleio.h"
#include "io/fileio.h"
#include "parser/parser.h"
#include "CLI/CLI.hpp"
#include "utils.h"


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

    // Set terminal to raw mode
    enableRawConsoleMode();

    int exitCode = 1;
    try {
        std::vector<RawFile> programLines;
        programLines.reserve(inputFileNames.size()); // Preallocate memory for performance
        for (const std::string& fileName : inputFileNames)
            programLines.push_back({getFileBasename(fileName), readFileLines(fileName)});

        const std::vector<SourceLine> program = Tokenizer::tokenize(programLines);

        Parser parser{};
        const MemLayout layout = parser.parse(program);

        Interpreter interpreter{};
        exitCode = interpreter.interpret(layout);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    // Restore terminal settings
    disableRawConsoleMode();

    return exitCode;
}
