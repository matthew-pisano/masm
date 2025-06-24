//
// Created by matthew on 6/16/25.
//

#ifndef DEBUG_INFO_H
#define DEBUG_INFO_H
#include <memory>
#include <string>


/**
 * Struct representing the location of a source line in the program
 */
struct SourceLocator {
    std::string filename;
    size_t lineno;
    std::string text;
};


/**
 * A structure that contains debug information for a specific instruction or source line
 */
struct DebugInfo {
    /**
     * The source file locator for the instruction, which includes the filename and line number
     */
    std::shared_ptr<SourceLocator> source;

    /**
     * The label assigned to the given data, if any
     */
    std::string label;
};

#endif // DEBUG_INFO_H
