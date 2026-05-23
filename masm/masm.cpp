#include <iostream>
#include <string>
#include <vector>

#include <CLI/CLI.hpp>

#include <masm/assembler/parser.hpp>
#include <masm/assembler/serialization.hpp>
#include <masm/io/consoleio.hpp>

#include "fileio.hpp"
#include "load_layout.hpp"
#include "version.h"


int main(const int argc, char* argv[]) {
    std::string name = "masm";
    const std::string _computedVersionString(Version::VERSION);
    const std::string version = name + " " + _computedVersionString;

    std::vector<std::string> inputFileNames;
    bool useLittleEndian = false;
    bool debugBuild = false;
    bool saveTemps = false;
    std::string outputFileName;

    CLI::App app{version + " - MIPS Assembler", name};
    app.add_option("file", inputFileNames, "A MIPS assembly file")->required()->allow_extra_args();
    app.add_flag("-l,--little-endian", useLittleEndian,
                 "Use little-endian byte order for memory layout (default is big-endian)");
    app.add_flag("-g", debugBuild, "Whether to generate a debug build");
    app.add_flag("--save-temps", saveTemps, "Write intermediate files to the current working directory");
    app.add_option("-o", outputFileName, "The name of the output file");
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

    // Resolve absolute file names
    for (size_t i = 0; i < inputFileNames.size(); i++)
        try {
            inputFileNames[i] = std::filesystem::canonical(inputFileNames[i]).string();
        } catch (std::filesystem::filesystem_error&) {
            std::cerr << "error: Could not find file '" << inputFileNames[i] << "'" << std::endl;
            return 1;
        }

    // Ensure the directory of the output file exists
    if (!outputFileName.empty()) {
        std::filesystem::path outputFilePath(outputFileName);
        std::string outputDirName = std::filesystem::absolute(outputFilePath).parent_path().string();
        if (!std::filesystem::is_directory(outputDirName)) {
            std::cerr << "error: Could not find directory '" << outputDirName << "'" << std::endl;
            return 1;
        }
    } else
        outputFileName = std::filesystem::path(inputFileNames[0]).stem().string();

    int exitCode = 1;
    try {
        Parser parser(useLittleEndian);
        const MemLayout layout = loadLayoutFromSource(inputFileNames, parser);
        if (saveTemps) {
            const std::string preprocessed = stringifyLayout(layout, parser.getLabels());
            writeFile(outputFileName + ".i", preprocessed);
        }

        const std::vector<std::byte> binary = saveLayout(layout, debugBuild);
        writeFileBytes(outputFileName + ".o", binary);

        exitCode = 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    // Restore terminal settings
    conHandle.disableRawConsoleMode();

    return exitCode;
}
