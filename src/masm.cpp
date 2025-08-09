#include <iostream>
#include <string>
#include <vector>

#include "CLI/CLI.hpp"
#include "debug/intermediates.h"
#include "interpreter/interpreter.h"
#include "io/consoleio.h"
#include "io/fileio.h"
#include "parser/parser.h"
#include "runtime.h"
#include "utils.h"
#include "version.h"


int main(const int argc, char* argv[]) {
    std::string name = "masm";
    const std::string _computedVersionString(Version::VERSION);
    const std::string version = name + " " + _computedVersionString;

    std::vector<std::string> inputFileNames;
    bool useMMIO = false;
    bool useLittleEndian = false;
    bool saveTemps = false;
    bool assembleOnly = false;

    CLI::App app{version + " - MIPS Interpreter", name};
    app.add_option("file", inputFileNames, "A MIPS assembly file")->required()->allow_extra_args();
    app.add_flag("-m,--mmio", useMMIO,
                 "Use memory-mapped I/O instead of system calls for input/output operations");
    app.add_flag("-l,--little-endian", useLittleEndian,
                 "Use little-endian byte order for memory layout (default is big-endian)");
    app.add_flag("--save-temps", saveTemps,
                 "Write intermediate files to the current working directory");
    app.add_flag("-s,--assemble", assembleOnly, "Assemble only; do not execute the given program");
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

    const bool loadingBinary = isLoadingBinary(inputFileNames);
    if (loadingBinary && saveTemps)
        std::cerr << "Warning: temp files are not generated when parsing binaries" << std::endl;
    if (loadingBinary && useLittleEndian)
        std::cerr << "Warning: little-endian mode has no effect on binary files" << std::endl;

    int exitCode = 1;
    try {
        MemLayout layout;
        if (loadingBinary)
            layout = loadLayoutFromBinary(inputFileNames);
        else {
            Parser parser(useLittleEndian);
            layout = loadLayoutFromSource(inputFileNames, parser);
            if (saveTemps) {
                std::string preprocessed = stringifyLayout(layout, parser.getLabels());
                writeFile(inputFileNames[0] + ".i", preprocessed);

                std::vector<std::byte> binary = saveLayout(layout);
                writeFileBytes(inputFileNames[0] + ".o", binary);
            }
        }

        // Do not interpret program if only assembling
        if (!assembleOnly) {
            const IOMode ioMode = useMMIO ? IOMode::MMIO : IOMode::SYSCALL;
            Interpreter interpreter(ioMode, conHandle, useLittleEndian);
            exitCode = interpreter.interpret(layout);
        } else
            exitCode = 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    // Restore terminal settings
    conHandle.disableRawConsoleMode();

    return exitCode;
}
