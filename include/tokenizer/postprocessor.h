//
// Created by matthew on 5/9/25.
//

#ifndef POSTPROCESSOR_H
#define POSTPROCESSOR_H
#include <string>
#include <vector>

#include "tokenizer.h"


class Postprocessor {

    /**
     * Adds all global declarations to the globals vector and removes declarations from file
     * @param globals The vector to add global declarations to along with their original lines
     * @param tokenizedFile The tokenized file to collect and prune globals from
     */
    static void collectGlobals(std::vector<std::pair<std::string, SourceLine>>& globals,
                               std::vector<SourceLine>& tokenizedFile);

    /**
     * A helper function that mangles labels in the given line of tokens
     * @param globals The vector of global labels to check against
     * @param lineTokens The line of tokens to mangle
     * @param fileId The file ID to append to the label
     * @return The label declaration found, if any
     */
    static std::string mangleLabelsInLine(std::vector<std::string>& globals, SourceLine& lineTokens,
                                          const std::string& fileId);

    /**
     * A helper function that parses the parameters of a macro into a smaller vector of tokens
     * @param line The macro declaration line of tokens to parse
     * @return The parsed macro parameters
     */
    static std::vector<Token> parseMacroParams(const SourceLine& line);

    /**
     * A struct representing a macro
     */
    struct Macro {
        std::string name;
        std::vector<Token> params;
        std::vector<SourceLine> body;
    };

    /**
     * A helper function that expands a macro in the given tokenized file
     * @param macro The macro to expand
     * @param pos The position in the tokenized file to expand the macro at
     * @param tokenizedFile The tokenized file to expand the macro in
     */
    static void expandMacro(const Macro& macro, size_t& pos,
                            std::vector<SourceLine>& tokenizedFile);

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
    static void replaceEqv(std::vector<SourceLine>& tokenizedFile);

    /**
     * Modifies the give token line to replace the pattern of y($xx) with $xx, y to match MIPS
     * addressing mode when a close paren is reached
     * @param tokenizedFile The tokenized file to replace base addressing syntax in
     */
    static void processBaseAddressing(std::vector<SourceLine>& tokenizedFile);

    /**
     * Expands macros in the given tokenized file
     * @param tokenizedFile The tokenized file to expand macros in
     */
    static void processMacros(std::vector<SourceLine>& tokenizedFile);

    /**
     * Name mangels tokens in the given program map by adding the file ID to the label
     * @param programMap The map of file IDs to their tokenized lines
     * @throw MasmSyntaxError When the file ID is empty
     */
    static void mangleLabels(std::map<std::string, std::vector<SourceLine>>& programMap);

    /**
     * Processes the includes in the given program map by replacing them with the corresponding file
     * @param rawProgramMap The map of file IDs to their tokenized lines
     */
    static void processIncludes(std::map<std::string, std::vector<SourceLine>>& rawProgramMap);
};

#endif // POSTPROCESSOR_H
