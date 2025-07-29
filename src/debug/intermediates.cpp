//
// Created by matthew on 7/28/25.
//

#include "debug/intermediates.h"

#include "tokenizer/postprocessor.h"


std::string memSectionToName(const MemSection& section) {
    switch (section) {
        case MemSection::DATA:
            return "data";
        case MemSection::HEAP:
            return "heap";
        case MemSection::GLOBAL:
            return "global";
        case MemSection::STACK:
            return "stack";
        case MemSection::TEXT:
            return "text";
        case MemSection::KTEXT:
            return "ktext";
        case MemSection::KDATA:
            return "kdata";
        case MemSection::MMIO:
            return "mmio";
    }
    throw std::runtime_error("Unknown memory section");
}


std::string layoutAsString(const MemLayout& layout, const LabelMap& labelMap) {
    std::string program;

    for (const auto& [section, data] : layout.data) {
        const uint32_t sectionOffset = memSectionOffset(section);
        program += "." + memSectionToName(section) + "\n";

        for (uint32_t i = 0; i < data.size(); i += 4) {
            uint32_t address = sectionOffset + i;
            DebugInfo debugInfo = layout.debugInfo.at(address);

            // Add label if it exists
            try {
                program += unmangleLabel(labelMap.lookupLabel(address)) + ":\n";
            } catch (const std::runtime_error&) {
            }

            // Add instruction or data
            program += debugInfo.source.text + "\n";
        }
    }

    return program;
}

std::vector<std::byte> layoutAsBinary(const MemLayout& layout) { return {}; }
