//
// Created by matthew on 4/13/25.
//

#include "fileio.h"

#include <fstream>
#include <stdexcept>


std::vector<std::string> readFileLines(const std::string& fileName) {
    std::ifstream inputFile;
    std::vector<std::string> result;

    inputFile.open(fileName);

    if (inputFile.is_open()) {
        std::string line;
        // Read data from the file line by line
        while (getline(inputFile, line))
            result.push_back(line);

        // Close the file
        inputFile.close();
    } else
        throw std::runtime_error("Could not open file");

    return result;
}
