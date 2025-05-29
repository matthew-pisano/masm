//
// Created by matthew on 4/13/25.
//

#include "io/fileio.h"

#include <fstream>
#include <stdexcept>


std::string readFile(const std::string& fileName) {
    std::ifstream inputFile;
    std::string result;

    inputFile.open(fileName);

    if (!inputFile.is_open())
        throw std::runtime_error("Could not open file " + fileName);

    // Read data from the file character by character
    char ch;
    while (inputFile.get(ch))
        result.push_back(ch);

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
