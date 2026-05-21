//
// Created by matthew on 4/13/25.
//

#ifndef FILEIO_H
#define FILEIO_H

#include <fstream>
#include <glob.h>
#include <string>
#include <vector>


/**
 * Resolves wildcard characters in file paths to actual file names
 * @param rawPaths A vector of file paths that may contain wildcard characters
 * @return A vector of file paths with wildcards resolved to actual file names
 */
inline std::vector<std::string> resolveWildcards(const std::vector<std::string>& rawPaths) {
    std::vector<std::string> resolvedPaths;
    for (const auto& path : rawPaths) {
        // Use glob to resolve wildcards
        glob_t globResult{};
        if (glob(path.c_str(), GLOB_TILDE, nullptr, &globResult) == 0) {
            for (size_t i = 0; i < globResult.gl_pathc; ++i)
                resolvedPaths.emplace_back(globResult.gl_pathv[i]);
            globfree(&globResult);
        } else
            // If glob fails, add the original path
            resolvedPaths.push_back(path);
    }
    return resolvedPaths;
}

/**
 * Loads a file and returns its contents as a string
 * @param fileName The name of the file to load
 * @return A string containing the contents of the file
 * @throw runtime_error When a file fails to open
 */
inline std::string readFile(const std::string& fileName) {
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


/**
 * Loads a file and returns its contents as a vector of bytes (characters)
 * @param fileName The name of the file to load
 * @return A vector of the bytes within the given file
 */
inline std::vector<std::byte> readFileBytes(const std::string& fileName) {
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


/**
 * Writes a string to a file, overwriting any existing contents
 * @param fileName The name of the file to write to
 * @param contents The contents to write to the file
 */
inline void writeFile(const std::string& fileName, const std::string& contents) {
    std::ofstream outputFile;

    outputFile.open(fileName, std::ios::out | std::ios::trunc);

    if (!outputFile.is_open())
        throw std::runtime_error("Could not open file " + fileName);

    // Write the contents to the file
    outputFile << contents;

    // Close the file
    outputFile.close();
}


/**
 * Writes a vector of bytes to a binary file, overwriting any existing contents
 * @param fileName The name of the file to write to
 * @param contents The contents to write to the file as a vector of bytes
 */
inline void writeFileBytes(const std::string& fileName, const std::vector<std::byte>& contents) {
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

#endif // FILEIO_H
