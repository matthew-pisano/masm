#include <iostream>
#include <string>
#include <vector>

#include "CLI/CLI.hpp"
#include "interpreter/interpreter.h"
#include "io/consoleio.h"
#include "io/fileio.h"
#include "parser/parser.h"
#include "utils.h"


int main(const int argc, char* argv[]) {
    std::string version = "masm 1.1.0";

    std::vector<std::string> inputFileNames;
    bool useMMIO = false;

    CLI::App app{version + " - MIPS Interpreter", "masm"};
    app.add_option("file", inputFileNames, "A MIPS assembly file")->required()->allow_extra_args();
    app.add_flag("--mmio", useMMIO,
                 "Use memory-mapped I/O instead of system calls for input/output operations");
    app.set_version_flag("--version", version);

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        return app.exit(e);
    }

    ConsoleHandle conHandle;
    // Set terminal to raw mode
    conHandle.enableRawConsoleMode();

    int exitCode = 1;
    try {
        std::vector<SourceFile> programLines;
        programLines.reserve(inputFileNames.size()); // Preallocate memory for performance
        for (const std::string& fileName : inputFileNames)
            programLines.push_back({getFileBasename(fileName), readFile(fileName)});

        const std::vector<LineTokens> program = Tokenizer::tokenize(programLines);

        Parser parser{};
        const MemLayout layout = parser.parse(program);

        const IOMode ioMode = useMMIO ? IOMode::MMIO : IOMode::SYSCALL;
        Interpreter interpreter(ioMode, conHandle);
        exitCode = interpreter.interpret(layout);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    // Restore terminal settings
    conHandle.disableRawConsoleMode();

    return exitCode;
}
