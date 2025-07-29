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


void writeFile(const std::string& fileName, const std::string& contents) {
    std::ofstream outputFile;

    outputFile.open(fileName, std::ios::out | std::ios::trunc);

    if (!outputFile.is_open())
        throw std::runtime_error("Could not open file " + fileName);

    // Write the contents to the file
    outputFile << contents;

    // Close the file
    outputFile.close();
}


void writeFileBytes(const std::string& fileName, const std::vector<std::byte>& contents) {
    std::ofstream outputFile;

    outputFile.open(fileName, std::ios::out | std::ios::binary | std::ios::trunc);

    if (!outputFile.is_open())
        throw std::runtime_error("Could not open file " + fileName);

    // Write the contents to the file byte by byte
    for (const auto& byte : contents)
        outputFile.write(reinterpret_cast<const char*>(&byte), sizeof(byte));

    // Close the file
    outputFile.close();
}
