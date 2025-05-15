#include <iostream>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <vector>

#include "CLI/CLI.hpp"
#include "fileio.h"
#include "interpreter.h"
#include "parser.h"


// Function to change terminal settings
void enableRawMode() {
    termios term;
    tcgetattr(STDIN_FILENO, &term);
    // Turn off canonical mode and echo mode
    term.c_lflag &= ~(ICANON);
    // Set minimum number of input bytes and timeout
    term.c_cc[VMIN] = 1; // Wait for at least one byte
    term.c_cc[VTIME] = 0; // No timeout
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &term);
}


// Function to restore terminal settings
void disableRawMode() {
    termios term;
    tcgetattr(STDIN_FILENO, &term);

    // Restore canonical mode and echo mode
    term.c_lflag |= (ICANON);

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &term);
}


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
    enableRawMode();

    int exitCode = 1;
    try {
        std::vector<std::vector<std::string>> programLines;
        programLines.reserve(inputFileNames.size()); // Preallocate memory for performance
        for (const std::string& fileName : inputFileNames)
            programLines.push_back(readFileLines(fileName));

        const std::vector<std::vector<Token>> program = Tokenizer::tokenize(programLines);

        Parser parser{};
        const MemLayout layout = parser.parse(program);

        Interpreter interpreter{};
        exitCode = interpreter.interpret(layout);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    // Restore terminal settings
    disableRawMode();

    return exitCode;
}
