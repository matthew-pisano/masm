//
// Created by matthew on 4/26/25.
//

#ifndef LABELS_H
#define LABELS_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "tokenizer/tokenizer.h"

/**
 * Class to manage the mapping of labels to memory locations
 */
class LabelMap {

    /**
     * A map between the names of labels and their associated memory addresses
     */
    std::map<std::string, uint32_t> labelMap;

public:
    /**
     * Modifies instruction arguments to replace label references with labeled memory locations
     * @param instructionArgs The instruction arguments to modify
     * @throw runtime_error When one of the arguments references an unknown label
     */
    void resolveLabels(std::vector<Token>& instructionArgs);

    /**
     * Looks up the first label corresponding to an address
     * @param address The address to look up
     * @return The label corresponding to the address
     * @throw runtime_error When no label is found for the address
     */
    [[nodiscard]] std::string lookupLabel(uint32_t address) const;

    /**
     * Populates the label map prior to processing using static allocations for the given tokens
     * @param tokens The program tokens
     * @throw MasmSyntaxError When a duplicate label definition is detected
     */
    void populateLabelMap(const std::vector<LineTokens>& tokens);

    /**
     * Checks if the label map contains a label
     * @param label The label to check for
     * @return True if the label exists, false otherwise
     */
    bool contains(const std::string& label) const;

    [[nodiscard]] uint32_t operator[](const std::string& label) const { return labelMap.at(label); }

    uint32_t& operator[](const std::string& label) { return labelMap[label]; }
};

#endif // LABELS_H
