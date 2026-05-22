//
// Created by matthew on 4/13/25.
//

#ifndef FILEIO_H
#define FILEIO_H

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>


/**
 * Expands leading '~' to the user's home directory
 * @param path A raw file path
 * @return The filepath with the leading tilde replaced with the home directory
 */
inline std::string expandTilde(const std::string& path) {
    if (path.empty() || path[0] != '~')
        return path;
#ifdef _WIN32
    const char* home = getenv("USERPROFILE");
#else
    const char* home = getenv("HOME");
#endif
    return home ? std::string(home) + path.substr(1) : path;
}

/**
 * Matches a filename against a wildcard pattern supporting '*' and '?'
 * Case-insensitive on Windows, case-sensitive elsewhere
 *
 * @param pattern The wildcard pattern to match against
 * @param path The path to match
 * @return Whether the given path matches the pattern
 */
inline bool wildcardMatch(const std::string& pattern, const std::string& path) {
    // Normalize strings to lower case on Windows
    auto normalizeCase = [](std::string s) {
#ifdef _WIN32
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
#endif
        return s;
    };

    const std::string normPattern = normalizeCase(pattern);
    const std::string normPath = normalizeCase(path);

    size_t patternIdx = 0;
    size_t pathIdx = 0;
    size_t lastStar = std::string::npos;
    size_t lastMatch = 0;

    while (pathIdx < normPath.size()) {
        // Match a single character
        const bool characterMatch = normPattern[patternIdx] == '?' || normPattern[patternIdx] == normPath[pathIdx];
        if (patternIdx < normPattern.size() && characterMatch) {
            ++patternIdx;
            ++pathIdx;
        }
        // Match a group of characters
        else if (patternIdx < normPattern.size() && normPattern[patternIdx] == '*') {
            lastStar = patternIdx++;
            lastMatch = pathIdx;
        } else if (lastStar != std::string::npos) {
            patternIdx = lastStar + 1;
            pathIdx = ++lastMatch;
        } else
            return false;
    }

    // Skip over any remaining wildcards
    while (patternIdx < normPattern.size() && normPattern[patternIdx] == '*')
        ++patternIdx;

    return patternIdx == normPattern.size();
}

/**
 * Resolves wildcard characters in file paths to actual file names, supports '*' and '?' wildcards, and '~' home
 * directory expansion
 *
 * @param rawPaths A vector of file paths that may contain wildcard characters
 * @return A vector of resolved file paths, sorted alphabetically
 */
inline std::vector<std::string> resolveWildcards(const std::vector<std::string>& rawPaths) {
    std::vector<std::string> resolvedPaths;

    for (const auto& rawPath : rawPaths) {
        const std::string pathStr = expandTilde(rawPath);
        const std::filesystem::path path(pathStr);
        const std::filesystem::path dir = path.parent_path();
        const std::string pattern = path.filename().string();

        // No wildcard is in the string
        if (pattern.find_first_of("*?") == std::string::npos) {
            resolvedPaths.push_back(pathStr);
            continue;
        }

        const std::filesystem::path searchDir = dir.empty() ? std::filesystem::path(".") : dir;

        std::error_code ec;
        std::vector<std::string> matches;

        for (const auto& entry : std::filesystem::directory_iterator(searchDir, ec)) {
            const std::string filename = entry.path().filename().string();
            if (wildcardMatch(pattern, filename))
                matches.push_back(entry.path().string());
        }

        if (ec || matches.empty()) {
            resolvedPaths.push_back(rawPath);
        } else {
            // Sort results if matched
            std::ranges::sort(matches);
            resolvedPaths.insert(resolvedPaths.end(), matches.begin(), matches.end());
        }
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
