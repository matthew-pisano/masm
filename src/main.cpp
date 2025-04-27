

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
        std::map<std::string, std::vector<std::vector<Token>>> programMap;
        Tokenizer tokenizer{};
        for (const std::string& fileName : inputFileNames) {
            std::string baseFileName = fileName.contains('/')
                                               ? fileName.substr(fileName.find_last_of('/') + 1)
                                               : fileName;
            baseFileName = std::regex_replace(baseFileName, std::regex(R"([\.-])"), "_");

            const std::vector<std::string> lines = readFileLines(fileName);
            programMap[baseFileName] = tokenizer.tokenize(lines);
        }

        // Mangle labels if there is more than one file
        if (programMap.size() > 1)
            tokenizer.mangleLabels(programMap);

        std::vector<std::vector<Token>> program;
        for (std::pair<const std::string, std::vector<std::vector<Token>>>& programFile :
             programMap)
            program.insert(program.end(), programFile.second.begin(), programFile.second.end());

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
