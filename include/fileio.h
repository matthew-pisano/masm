//
// Created by matthew on 4/13/25.
//

#ifndef FILEIO_H
#define FILEIO_H

#include <string>
#include <vector>

/**
 * Loads a file and returns its contents as a vector of strings, where each string is a line in the file.
 * @param fileName The name of the file to load
 * @return A vector of the lines within the given file
 * @throw runtime_error When a file fails to open
 */
std::vector<std::string> readFileLines(const std::string& fileName);

#endif // FILEIO_H
