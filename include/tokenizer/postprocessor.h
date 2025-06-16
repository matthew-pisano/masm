//
// Created by matthew on 5/9/25.
//

#ifndef POSTPROCESSOR_H
#define POSTPROCESSOR_H
#include <map>
#include <string>
#include <vector>

#include "tokenizer.h"


/**
 * Mangles a label by adding the file ID to it, ensuring that labels are unique across files.
 * @param label The label to mangle
 * @param filename The name of the file the label is in, used to create a unique identifier
 * @return The mangled label
 */
std::string mangleLabel(const std::string& label, const std::string& filename);


/**
 * Mangles a label within a macro by adding the filename, macro name, and macro location to it
 * @param label The label to mangle
 * @param filename The name of the file the label is in
 * @param macroname The name of the macro the label is in
 * @param pos The position of the macro in the file
 * @return The mangled label
 */
std::string mangleMacroLabel(const std::string& label, const std::string& filename,
                             const std::string& macroname, size_t pos);


/**
 * Unmangles a label by removing the file ID from it, restoring the original label name.
 * @param mangledLabel The mangled label to unmangle
 * @return The unmangled label
 */
std::string unmangleLabel(const std::string& mangledLabel);


/**
 * A class that post-processes a tokenized file to perform various transformations like managing
 * global definitions and defining/expanding macros
 */
class Postprocessor {

    /**
     * Adds all global declarations to the globals vector and removes declarations from file
     * @param globals The vector to add global declarations to, along with their original lines
     * @param tokenizedFile The tokenized file to collect and prune globals from
     */
    static void collectGlobals(std::vector<std::pair<std::string, LineTokens>>& globals,
                               std::vector<LineTokens>& tokenizedFile);

    /**
     * A helper function that mangles labels in the given line of tokens
     * @param globals The vector of global labels to check against (skipped by mangling)
     * @param lineTokens The line of tokens to mangle
     * @param fileId The file ID to append to the label
     * @return The label declaration found, if any
     */
    static std::string mangleLabelsInLine(std::vector<std::string>& globals, LineTokens& lineTokens,
                                          const std::string& fileId);

    /**
     * A helper function that parses the parameters of a macro into a smaller vector of tokens
     * @param line The macro declaration line of tokens to parse
     * @return The parsed macro parameters
     */
    static std::vector<Token> parseMacroParams(const LineTokens& line);

    /**
     * A struct representing a macro
     */
    struct Macro {
        /**
         * The name of the macro
         */
        std::string name;

        /**
         * The parameters of the macro, if any
         */
        std::vector<Token> params;

        /**
         * The original line of tokens that declared the macro
         */
        std::vector<LineTokens> body;

        /**
         * The filename the macro was defined in
         */
        std::string filename;
    };

    /**
     * A helper function that expands a macro in the given tokenized file
     * @param macro The macro to expand
     * @param pos The position in the tokenized file to expand the macro at
     * @param tokenizedFile The tokenized file to expand the macro in
     */
    static void expandMacro(const Macro& macro, size_t& pos,
                            std::vector<LineTokens>& tokenizedFile);

    /**
     * A helper function that mangles the labels in a macro
     * @param macro The macro to mangle
     * @param pos The position in the tokenized file to mangle the macro at
     * @return A new macro with mangled labels
     */
    static Macro mangleMacroLabels(const Macro& macro, size_t pos);

public:
    /**
     * Replaces all eqv directives with the corresponding value
     * @param tokenizedFile The tokenized file to replace eqv directives in
     */
    static void replaceEqv(std::vector<LineTokens>& tokenizedFile);

    /**
     * Modifies the give token line to replace the pattern of y($xx) with $xx, y to match MIPS
     * addressing mode when a close paren is reached
     * @param tokenizedFile The tokenized file to replace base addressing syntax in
     */
    static void processBaseAddressing(std::vector<LineTokens>& tokenizedFile);

    /**
     * Expands macros in the given tokenized file
     * @param tokenizedFile The tokenized file to expand macros in
     */
    static void processMacros(std::vector<LineTokens>& tokenizedFile);

    /**
     * Name mangels tokens in the given program map by adding the file ID to the label
     * @param programMap The map of file IDs to their tokenized lines
     * @throw MasmSyntaxError When the file ID is empty
     */
    static void mangleLabels(std::map<std::string, std::vector<LineTokens>>& programMap);

    /**
     * Processes the includes in the given program map by replacing them with the corresponding file
     * @param rawProgramMap The map of file IDs to their tokenized lines
     */
    static void processIncludes(std::map<std::string, std::vector<LineTokens>>& rawProgramMap);
};

#endif // POSTPROCESSOR_H
