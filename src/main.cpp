#include <iostream>
#include <string>
#include <vector>

#include "CLI/CLI.hpp"
#include "interpreter/interpreter.h"
#include "io/consoleio.h"
#include "io/fileio.h"
#include "parser/parser.h"
#include "utils.h"
#include "version.h"


int main(const int argc, char* argv[]) {
    std::string name = "masm";
    std::string _computedVersionString(Version::VERSION);
    std::string version = name + " " + _computedVersionString;

    std::vector<std::string> inputFileNames;
    bool useMMIO = false;
    bool useLittleEndian = false;

    CLI::App app{version + " - MIPS Interpreter", name};
    app.add_option("file", inputFileNames, "A MIPS assembly file")->required()->allow_extra_args();
    app.add_flag("-m,--mmio", useMMIO,
                 "Use memory-mapped I/O instead of system calls for input/output operations");
    app.add_flag("-l,--little-endian", useLittleEndian,
                 "Use little-endian byte order for memory layout (default is big-endian)");
    app.set_version_flag("--version", version);

    // Set up help message
    app.failure_message([name](const CLI::App* _app, const CLI::Error& e) -> std::string {
        return name + ": error: " + CLI::FailureMessage::simple(_app, e);
    });

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
        std::vector<SourceFile> sourceFiles;
        sourceFiles.reserve(inputFileNames.size()); // Preallocate memory for performance
        for (const std::string& fileName : inputFileNames)
            sourceFiles.push_back({getFileBasename(fileName), readFile(fileName)});

        const std::vector<LineTokens> program = Tokenizer::tokenize(sourceFiles);

        Parser parser;
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
