

#include <iostream>
#include <string>
#include <vector>

#include "CLI/CLI.hpp"
#include "fileio.h"


int main(const int argc, char* argv[]) {

    std::string inputFile;

    CLI::App app{"A CPP MIPS Interpreter", "mipscpp"};
    app.add_option("input-file", inputFile, "Input file to load")->required();

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        return app.exit(e);
    }

    try {
        std::vector<std::string> lines = readFileLines(inputFile);
        for (const std::string& line : lines)
            std::cout << "> " << line << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
