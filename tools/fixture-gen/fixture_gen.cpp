//
// Created by matthew on 4/20/25.
//

#include <iostream>
#include <string>
#include <vector>

#include <CLI/CLI.hpp>

#include <masm/assembler/parser.hpp>
#include <masm/assembler/tokenizer.hpp>
#include <masm/simulator/simulator.hpp>
#include <masm/simulator/state.hpp>

#include "fileio.hpp"
#include "load_layout.hpp"


/**
 * Writes a tokenized representation of the given program lines to the given file handle
 * @param tokenFile The file handle to write to
 * @param programLines The lines of the program to write
 * @return The tokenized representation of the program lines as a vector of vectors
 */
std::vector<LineTokens> genTokenFile(std::ofstream& tokenFile, const std::vector<SourceFile>& programLines) {
    const std::vector<LineTokens> tokenizedLines = Tokenizer::tokenize(programLines);
    for (const LineTokens& tokenLine : tokenizedLines) {
        for (const Token& token : tokenLine.tokens) {
            constexpr unsigned char groupSep = 0x1d;
            std::string TokenCategory = std::to_string(static_cast<int>(token.category));
            if (static_cast<int>(token.category) < 10)
                TokenCategory.insert(0, "0");

            tokenFile << TokenCategory << token.value << groupSep;
        }
        tokenFile << std::endl;
    }

    return tokenizedLines;
}


/**
 * Writes a parser representation of the given tokenized lines to the given file handle
 * @param parserFile The file handle to write to
 * @param tokenizedLines The tokenized lines to parse
 * @param useLittleEndian Whether to use little-endian byte order
 * @param rawParse Whether to parse the file in raw mode (without adding any new tokens)
 */
void generateParserFile(std::ofstream& parserFile, const std::vector<LineTokens>& tokenizedLines,
                        const bool useLittleEndian, const bool rawParse) {
    Parser parser(useLittleEndian);
    MemLayout memLayout = parser.parse(tokenizedLines, rawParse);

    for (const auto& [memSection, bytes] : memLayout.data) {
        constexpr unsigned char groupSep = 0x1d;
        parserFile << groupSep << static_cast<unsigned char>(memSection);
        for (const std::byte byte : bytes)
            parserFile << static_cast<unsigned char>(byte);
    }
}


int main(const int argc, char* argv[]) {

    std::vector<std::string> inputFileNames;
    bool useLittleEndian = false;
    bool rawParse = false;
    bool noSim = false;
    bool useMMIO = false;

    CLI::App app{"libmasm Intermediate Generator", "libmasm-fg"};
    app.add_option("input-file", inputFileNames, "Input file to load")->required()->allow_extra_args();
    app.add_flag("-l,--little-endian", useLittleEndian,
                 "Use little-endian byte order for memory layout (default is big-endian)");
    app.add_flag("-m,--mmio", useMMIO, "Use memory-mapped I/O instead of system calls for input/output operations");
    app.add_flag("--no-sim", noSim, "Only generate static fixtures, do not simulate");
    app.add_flag("--raw-parse", rawParse, "Parses the raw text of the file without inserting any extra helper tokens");

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        return app.exit(e);
    }

    // resolve wildcards in path names to real paths
    inputFileNames = resolveWildcards(inputFileNames);

    std::string projectName = getFileBasename(inputFileNames[0]);
    // Split off extension
    const size_t extPos = projectName.find_last_of('.');
    if (extPos != std::string::npos)
        projectName.erase(extPos); // Remove extension if it exists

    try {
        std::vector<SourceFile> sourceFiles;
        sourceFiles.reserve(inputFileNames.size()); // Preallocate memory for performance
        for (const std::string& fileName : inputFileNames)
            sourceFiles.push_back({getFileBasename(fileName), readFile(fileName)});

        std::ofstream tokenFile;
        tokenFile.open(projectName + ".tkn");
        if (!tokenFile.is_open())
            throw std::runtime_error("Could not open file " + projectName + ".tkn");
        const std::vector<LineTokens> tokenizedLines = genTokenFile(tokenFile, sourceFiles);
        tokenFile.close();

        std::ofstream parserFile;
        parserFile.open(projectName + ".pse");
        if (!parserFile.is_open())
            throw std::runtime_error("Could not open file " + projectName + ".pse");
        generateParserFile(parserFile, tokenizedLines, useLittleEndian, rawParse);
        parserFile.close();

        Parser parser(useLittleEndian);
        MemLayout memLayout = parser.parse(tokenizedLines, rawParse);
        const std::string preprocessed = stringifyLayout(memLayout, parser.getLabels());
        writeFile(projectName + ".i", preprocessed);

        if (noSim)
            return 0;

        std::ofstream textFile;
        textFile.open(projectName + ".txt");
        if (!textFile.is_open())
            throw std::runtime_error("Could not open file " + projectName + ".txt");

        std::ifstream inputFile;
        std::istream* istream = &std::cin;
        if (std::filesystem::exists(projectName + ".in.txt")) {
            inputFile.open(projectName + ".in.txt");
            if (!inputFile.is_open())
                throw std::runtime_error("Could not open file " + projectName + ".in.txt");
            istream = &inputFile;
        }

        StreamHandle streamHandle(*istream, textFile);

        const IOMode ioMode = useMMIO ? IOMode::MMIO : IOMode::SYSCALL;
        Simulator simulator(ioMode, streamHandle, useLittleEndian);
        simulator.simulate(memLayout);
        textFile.close();

        if (inputFile.is_open())
            inputFile.close();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
