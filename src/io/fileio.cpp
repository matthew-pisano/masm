//
// Created by matthew on 4/13/25.
//

#include "io/fileio.h"

#include <fstream>
#include <stdexcept>


std::vector<std::string> readFileLines(const std::string& fileName) {
    std::ifstream inputFile;
    std::vector<std::string> result;

    inputFile.open(fileName);

    if (!inputFile.is_open())
        throw std::runtime_error("Could not open file " + fileName);

    std::string line;
    // Read data from the file line by line
    while (getline(inputFile, line))
        result.push_back(line);

    // Close the file
    inputFile.close();

    return result;
}


std::vector<std::byte> readFileBytes(const std::string& fileName) {
    std::ifstream inputFile;
    std::vector<std::byte> result;

    inputFile.open(fileName);

    if (!inputFile.is_open())
        throw std::runtime_error("Could not open file " + fileName);

    // Read data from the file byte by byte
    char byte;
    while (inputFile.read(&byte, sizeof(byte)))
        result.push_back(static_cast<std::byte>(byte));

    // Close the file
    inputFile.close();

    return result;
}
