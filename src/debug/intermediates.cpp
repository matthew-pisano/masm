//
// Created by matthew on 7/28/25.
//

#include "debug/intermediates.h"

#include <iomanip>

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
        program += "\n." + memSectionToName(section) + "\n\n";

        for (uint32_t i = 0; i < data.size(); i++) {
            if (isSectionExecutable(section) && i % 4 != 0)
                continue; // Only consider word aligned bytes in executable sections

            uint32_t address = sectionOffset + i;
            // Add label if it exists
            try {
                program += "\n" + unmangleLabel(labelMap.lookupLabel(address)) + ":\n";
            } catch (const std::runtime_error&) {
            }

            // Add instruction or data
            if (isSectionExecutable(section)) {
                DebugInfo debugInfo = layout.debugInfo.at(address);
                program += debugInfo.source.text + "\n";
            } else {
                // Output current word as hex string
                const int32_t byte = static_cast<int32_t>(data.at(i));
                std::stringstream ss;
                ss << std::hex << std::setfill('0') << std::setw(2) << byte;
                program += ".byte 0x" + ss.str() + "\n";
            }
        }
    }

    return program;
}

std::vector<std::byte> layoutAsBinary(const MemLayout& layout) { return {}; }
