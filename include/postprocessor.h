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
     * @param globals The vector to add global declarations to
     * @param tokenizedFile The tokenized file to collect and prune globals from
     */
    static void collectGlobals(std::vector<std::string>& globals,
                               std::vector<std::vector<Token>>& tokenizedFile);

    /**
     * A helper function that mangles labels in the given line of tokens
     * @param globals The vector of global labels to check against
     * @param lineTokens The line of tokens to mangle
     * @param fileId The file ID to append to the label
     * @return The label declaration found, if any
     */
    static std::string mangleLabelsInLine(std::vector<std::string>& globals,
                                          std::vector<Token>& lineTokens,
                                          const std::string& fileId);

    /**
     * A helper function that parses the parameters of a macro into a smaller vector of tokens
     * @param line The macro declaration line of tokens to parse
     * @return The parsed macro parameters
     */
    static std::vector<Token> parseMacroParams(const std::vector<Token>& line);

    /**
     * A struct representing a macro
     */
    struct Macro {
        std::string name;
        std::vector<Token> params;
        std::vector<std::vector<Token>> body;
    };

    static void expandMacro(const Macro& macro, int& i,
                            std::vector<std::vector<Token>>& tokenizedFile);

public:
    /**
     * Replaces all eqv directives with the corresponding value
     * @param tokenizedFile The tokenized file to replace eqv directives in
     */
    static void replaceEqv(std::vector<std::vector<Token>>& tokenizedFile);

    /**
     * Modifies the give token line to replace the pattern of y($xx) with $xx, y to match MIPS
     * addressing mode when a close paren is reached
     * @param tokenizedFile The tokenized file to replace base addressing syntax in
     */
    static void processBaseAddressing(std::vector<std::vector<Token>>& tokenizedFile);

    /**
     * Expands macros in the given tokenized file
     * @param tokenizedFile The tokenized file to expand macros in
     */
    static void processMacros(std::vector<std::vector<Token>>& tokenizedFile);

    /**
     * Name mangels tokens in the given program map by adding the file ID to the label
     * @param programMap The map of file IDs to their tokenized lines
     * @throw runtime_error When the file ID is empty
     */
    static void mangleLabels(std::map<std::string, std::vector<std::vector<Token>>>& programMap);
};

#endif // POSTPROCESSOR_H
