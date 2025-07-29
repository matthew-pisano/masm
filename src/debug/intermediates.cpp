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

std::vector<std::byte> layoutAsBinary(const MemLayout& layout) {
    // Offsets for text, data, ktext, kdata
    std::vector binary = {std::byte{'M'}, std::byte{'A'}, std::byte{'S'}, std::byte{'M'},
                          std::byte{0},   std::byte{0},   std::byte{0},   std::byte{0},
                          std::byte{0},   std::byte{0},   std::byte{0},   std::byte{0},
                          std::byte{0},   std::byte{0},   std::byte{0},   std::byte{0},
                          std::byte{0},   std::byte{0},   std::byte{0},   std::byte{0}};

    // Insert an offset value into the four bytes after the given index of the binary vector
    auto insertOffset = [&binary](const size_t i, const uint32_t offset) {
        binary[i] = static_cast<std::byte>(offset & 0xFF);
        binary[i + 1] = static_cast<std::byte>((offset >> 8) & 0xFF);
        binary[i + 2] = static_cast<std::byte>((offset >> 16) & 0xFF);
        binary[i + 3] = static_cast<std::byte>((offset >> 24) & 0xFF);
    };

    // Add section offsets to vector
    if (layout.data.contains(MemSection::TEXT)) {
        insertOffset(4, binary.size());
        for (const std::byte& byte : layout.data.at(MemSection::TEXT))
            binary.push_back(byte);
    }
    if (layout.data.contains(MemSection::DATA)) {
        insertOffset(8, binary.size());
        for (const std::byte& byte : layout.data.at(MemSection::DATA))
            binary.push_back(byte);
        // Ensure length of binary vector is a multiple of 4
        while (binary.size() % 4 != 0)
            binary.push_back(std::byte{0});
    }
    if (layout.data.contains(MemSection::KTEXT)) {
        insertOffset(12, binary.size());
        for (const std::byte& byte : layout.data.at(MemSection::KTEXT))
            binary.push_back(byte);
    }
    if (layout.data.contains(MemSection::KDATA)) {
        insertOffset(16, binary.size());
        for (const std::byte& byte : layout.data.at(MemSection::KDATA))
            binary.push_back(byte);
    }

    return binary;
}
