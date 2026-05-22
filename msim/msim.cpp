#include <iostream>
#include <string>
#include <vector>

#include <CLI/CLI.hpp>

#include <masm/interpreter/interpreter.hpp>
#include <masm/io/consoleio.hpp>

#include "fileio.hpp"
#include "load_layout.hpp"
#include "version.h"


int main(const int argc, char* argv[]) {
    std::string name = "msim";
    const std::string _computedVersionString(Version::VERSION);
    const std::string version = name + " " + _computedVersionString;

    std::vector<std::string> inputFileNames;
    bool useMMIO = false;
    bool useLittleEndian = false;

    CLI::App app{version + " - MIPS Simulator", name};
    app.add_option("file", inputFileNames, "A MIPS binary object file")->required();
    app.add_flag("-m,--mmio", useMMIO, "Use memory-mapped I/O instead of system calls for input/output operations");
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

    // resolve wildcards in path names to real paths
    inputFileNames = resolveWildcards(inputFileNames);

    int exitCode = 1;
    try {
        const MemLayout layout = loadLayoutFromBinary(inputFileNames);

        const IOMode ioMode = useMMIO ? IOMode::MMIO : IOMode::SYSCALL;
        Interpreter interpreter(ioMode, conHandle, useLittleEndian);
        exitCode = interpreter.interpret(layout);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    // Restore terminal settings
    conHandle.disableRawConsoleMode();

    return exitCode;
}
