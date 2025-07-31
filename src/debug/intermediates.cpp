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


std::string stringifyLayout(const MemLayout& layout, const LabelMap& labelMap) {
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

std::vector<std::byte> saveLayout(const MemLayout& layout) {
    // Offsets for text, data, ktext, kdata
    std::vector binary = {std::byte{'M'}, std::byte{'A'}, std::byte{'S'}, std::byte{'M'},
                          std::byte{0},   std::byte{0},   std::byte{0},   std::byte{0},
                          std::byte{0},   std::byte{0},   std::byte{0},   std::byte{0},
                          std::byte{0},   std::byte{0},   std::byte{0},   std::byte{0},
                          std::byte{0},   std::byte{0},   std::byte{0},   std::byte{0}};

    // Insert an offset value into the four bytes after the given index of the binary vector
    auto insertOffset = [&binary](const size_t i, const uint32_t offset) {
        binary[i] = static_cast<std::byte>(offset & 0xFF);
        binary[i + 1] = static_cast<std::byte>(offset >> 8 & 0xFF);
        binary[i + 2] = static_cast<std::byte>(offset >> 16 & 0xFF);
        binary[i + 3] = static_cast<std::byte>(offset >> 24 & 0xFF);
    };

    // Ensure length of binary vector is a multiple of 4
    auto padBinary = [&binary]() {
        while (binary.size() % 4 != 0)
            binary.push_back(std::byte{0});
    };

    // Add section offsets to vector
    if (layout.data.contains(MemSection::TEXT)) {
        insertOffset(4, binary.size());
        // Add size byte of section
        binary.insert(binary.end(), 4, std::byte{0});
        insertOffset(binary.size() - 4, layout.data.at(MemSection::TEXT).size());
        // Add text data to binary
        for (const std::byte& byte : layout.data.at(MemSection::TEXT))
            binary.push_back(byte);
        padBinary();
    }
    if (layout.data.contains(MemSection::DATA)) {
        insertOffset(8, binary.size());
        // Add size byte of section
        binary.insert(binary.end(), 4, std::byte{0});
        insertOffset(binary.size() - 4, layout.data.at(MemSection::DATA).size());
        // Add static data to binary
        for (const std::byte& byte : layout.data.at(MemSection::DATA))
            binary.push_back(byte);
        padBinary();
    }
    if (layout.data.contains(MemSection::KTEXT)) {
        insertOffset(12, binary.size());
        // Add size byte of section
        binary.insert(binary.end(), 4, std::byte{0});
        insertOffset(binary.size() - 4, layout.data.at(MemSection::KTEXT).size());
        // Add ktext data to binary
        for (const std::byte& byte : layout.data.at(MemSection::KTEXT))
            binary.push_back(byte);
        padBinary();
    }
    if (layout.data.contains(MemSection::KDATA)) {
        insertOffset(16, binary.size());
        // Add size byte of section
        binary.insert(binary.end(), 4, std::byte{0});
        insertOffset(binary.size() - 4, layout.data.at(MemSection::KDATA).size());
        // Add kdata to binary
        for (const std::byte& byte : layout.data.at(MemSection::KDATA))
            binary.push_back(byte);
        padBinary();
    }

    return binary;
}


MemLayout loadLayout(const std::vector<std::byte>& binary) {
    // Check if the binary starts with the MASM magic number
    if (binary[0] != std::byte{'M'} || binary[1] != std::byte{'A'} || binary[2] != std::byte{'S'} ||
        binary[3] != std::byte{'M'}) {
        throw std::runtime_error("Invalid MASM binary format");
    }

    auto extractOffset = [&binary](const size_t index) {
        return static_cast<uint32_t>(binary.at(index)) |
               static_cast<uint32_t>(binary.at(index + 1)) << 8 |
               static_cast<uint32_t>(binary.at(index + 2)) << 16 |
               static_cast<uint32_t>(binary.at(index + 3)) << 24;
    };

    const std::vector<size_t> secHeaders = {extractOffset(4), extractOffset(8), extractOffset(12),
                                            extractOffset(16)};
    MemLayout layout;

    if (secHeaders[0] > 0) {
        const size_t textSize = extractOffset(secHeaders[0]);
        layout.data[MemSection::TEXT] = {};
        for (size_t i = 0; i < textSize; i++)
            layout.data[MemSection::TEXT].push_back(binary.at(secHeaders[0] + 4 + i));
    }
    if (secHeaders[1] > 0) {
        const size_t dataSize = extractOffset(secHeaders[1]);
        layout.data[MemSection::DATA] = {};
        for (size_t i = 0; i < dataSize; i++)
            layout.data[MemSection::DATA].push_back(binary.at(secHeaders[1] + 4 + i));
    }
    if (secHeaders[2] > 0) {
        const size_t ktextSize = extractOffset(secHeaders[2]);
        layout.data[MemSection::KTEXT] = {};
        for (size_t i = 0; i < ktextSize; i++)
            layout.data[MemSection::KTEXT].push_back(binary.at(secHeaders[2] + 4 + i));
    }
    if (secHeaders[3] > 0) {
        const size_t kdataSize = extractOffset(secHeaders[3]);
        layout.data[MemSection::KDATA] = {};
        for (size_t i = 0; i < kdataSize; i++)
            layout.data[MemSection::KDATA].push_back(binary.at(secHeaders[3] + 4 + i));
    }

    return layout;
}
