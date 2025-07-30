#include <iostream>
#include <string>
#include <vector>

#include "CLI/CLI.hpp"
#include "debug/intermediates.h"
#include "interpreter/interpreter.h"
#include "io/consoleio.h"
#include "io/fileio.h"
#include "parser/parser.h"
#include "utils.h"
#include "version.h"


/**
 * Loads a memory layout from source files, which are MIPS assembly files
 * @param inputFileNames A vector of file names to load the MIPS assembly source code from
 * @param parser The parser to use for parsing the source code
 * @return A memory layout object constructed from the source files
 */
MemLayout loadLayoutFromSource(const std::vector<std::string>& inputFileNames, Parser& parser) {
    std::vector<SourceFile> sourceFiles;
    sourceFiles.reserve(inputFileNames.size()); // Preallocate memory for performance
    for (const std::string& fileName : inputFileNames)
        sourceFiles.push_back({getFileBasename(fileName), readFile(fileName)});

    const std::vector<LineTokens> program = Tokenizer::tokenize(sourceFiles);
    const MemLayout layout = parser.parse(program);

    return layout;
}


/**
 * Loads a memory layout from a binary file, which is a compiled MIPS program
 * @param inputFileNames A vector of file names to load the binary data from
 * @return A memory layout object constructed from the binary data
 */
MemLayout loadLayoutFromBinary(const std::vector<std::string>& inputFileNames) {
    if (inputFileNames.size() > 1)
        throw std::runtime_error("Only one binary file may be loaded in at a time");

    const std::vector<std::byte> binary = readFileBytes(inputFileNames[0]);
    const MemLayout layout = loadLayout(binary);

    return layout;
}


/**
 * Checks if the first input file is a binary file (compiled MIPS program)
 * @param inputFileNames A vector of file names to check
 * @return True if the first file is a binary file, false otherwise
 */
bool isLoadingBinary(const std::vector<std::string>& inputFileNames) {
    if (inputFileNames.empty())
        return false;

    const std::string& firstName = inputFileNames[0];
    const size_t firstSize = firstName.size();
    return firstSize > 2 && firstName.substr(firstSize - 2, 2) == ".o";
}


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

    bool loadingBinary = isLoadingBinary(inputFileNames);
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
