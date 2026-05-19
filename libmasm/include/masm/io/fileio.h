//
// Created by matthew on 4/13/25.
//

#ifndef FILEIO_H
#define FILEIO_H

#include <string>
#include <vector>


/**
 * Resolves wildcard characters in file paths to actual file names
 * @param rawPaths A vector of file paths that may contain wildcard characters
 * @return A vector of file paths with wildcards resolved to actual file names
 */
std::vector<std::string> resolveWildcards(const std::vector<std::string>& rawPaths);

/**
 * Loads a file and returns its contents as a string
 * @param fileName The name of the file to load
 * @return A string containing the contents of the file
 * @throw runtime_error When a file fails to open
 */
std::string readFile(const std::string& fileName);


/**
 * Loads a file and returns its contents as a vector of bytes (characters)
 * @param fileName The name of the file to load
 * @return A vector of the bytes within the given file
 */
std::vector<std::byte> readFileBytes(const std::string& fileName);


/**
 * Writes a string to a file, overwriting any existing contents
 * @param fileName The name of the file to write to
 * @param contents The contents to write to the file
 */
void writeFile(const std::string& fileName, const std::string& contents);


/**
 * Writes a vector of bytes to a binary file, overwriting any existing contents
 * @param fileName The name of the file to write to
 * @param contents The contents to write to the file as a vector of bytes
 */
void writeFileBytes(const std::string& fileName, const std::vector<std::byte>& contents);

#endif // FILEIO_H
