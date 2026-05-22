//
// Created by matthew on 7/28/25.
//

#include <masm/assembler/serialization.hpp>

#include <iomanip>
#include <span>

#include "assembler/postprocessor.hpp"
#include "util/conversion.hpp"


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

uint32_t BEByteToi32(const std::span<const std::byte>& bytes) {
    uint32_t i32 = 0;
    i32 |= static_cast<uint32_t>(bytes[0]) << 24;
    i32 |= static_cast<uint32_t>(bytes[1]) << 16;
    i32 |= static_cast<uint32_t>(bytes[2]) << 8;
    i32 |= static_cast<uint32_t>(bytes[3]);
    return i32;
}

std::string bytesToString(const std::span<const std::byte>& bytes) {
    std::string string;

    for (const std::byte byte : bytes) {
        const char c = static_cast<char>(byte);
        if (c == 0)
            break; // Break on null-terminator
        string += c;
    }
    return string;
}

std::vector<std::byte> serializeDebugInfo(const std::map<uint32_t, DebugInfo>& debugInfo) {
    std::vector<std::byte> binaryDebugInfo;

    for (auto [lineAddr, lineDebugInfo] : debugInfo) {
        // Line address (4 bytes)
        std::vector<std::byte> lineAddrBytes = i32ToBEByte(lineAddr);
        binaryDebugInfo.insert(binaryDebugInfo.end(), lineAddrBytes.begin(), lineAddrBytes.end());
        // Filename (n bytes)
        std::vector<std::byte> filenameBytes = stringToBytes(lineDebugInfo.source.filename, true);
        binaryDebugInfo.insert(binaryDebugInfo.end(), filenameBytes.begin(), filenameBytes.end());
        // Line number (4 bytes)
        std::vector<std::byte> linenoBytes = i32ToBEByte(lineDebugInfo.source.lineno);
        binaryDebugInfo.insert(binaryDebugInfo.end(), linenoBytes.begin(), linenoBytes.end());
        // Line text (n bytes)
        std::vector<std::byte> textBytes = stringToBytes(lineDebugInfo.source.text, true);
        binaryDebugInfo.insert(binaryDebugInfo.end(), textBytes.begin(), textBytes.end());
        // Line label (n bytes)
        std::vector<std::byte> labelBytes = stringToBytes(lineDebugInfo.label, true);
        binaryDebugInfo.insert(binaryDebugInfo.end(), labelBytes.begin(), labelBytes.end());
    }

    return binaryDebugInfo;
}

std::map<uint32_t, DebugInfo> deSerializeDebugInfo(const std::vector<std::byte>& binaryDebugInfo) {
    std::map<uint32_t, DebugInfo> debugInfo;

    const std::span binarySpan(binaryDebugInfo);

    size_t byteIdx = 0;
    while (byteIdx < binaryDebugInfo.size()) {
        uint32_t lineAddr = BEByteToi32(binarySpan.subspan(byteIdx, 4));
        byteIdx += 4;
        std::string filename = bytesToString(binarySpan.subspan(byteIdx, binaryDebugInfo.size() - byteIdx));
        byteIdx += filename.length() + 1;
        uint32_t lineno = BEByteToi32(binarySpan.subspan(byteIdx, 4));
        byteIdx += 4;
        std::string text = bytesToString(binarySpan.subspan(byteIdx, binaryDebugInfo.size() - byteIdx));
        byteIdx += text.length() + 1;
        std::string label = bytesToString(binarySpan.subspan(byteIdx, binaryDebugInfo.size() - byteIdx));
        byteIdx += label.length() + 1;
        debugInfo[lineAddr] = {{filename, lineno, text}, label};
    }

    return debugInfo;
}

std::string stringifyLayout(const MemLayout& layout, const LabelMap& labelMap) {
    std::string program;

    for (const auto& [section, data] : layout.data) {
        const uint32_t sectionOffset = memSectionOffset(section);
        if (!program.empty())
            program += "\n\n";
        program += "." + memSectionToName(section) + "\n";

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
                if (!layout.debugInfo.contains(address))
                    throw std::runtime_error("No debug info at address " + std::to_string(address));

                const DebugInfo debugInfo = layout.debugInfo.at(address);
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

std::vector<std::byte> saveLayout(const MemLayout& layout, const bool debug) {
    // Offsets for text, data, ktext, kdata
    std::vector binary = {std::byte{'M'}, std::byte{'A'}, std::byte{'S'}, std::byte{'M'}, std::byte{0}, std::byte{0},
                          std::byte{0},   std::byte{0},   std::byte{0},   std::byte{0},   std::byte{0}, std::byte{0},
                          std::byte{0},   std::byte{0},   std::byte{0},   std::byte{0},   std::byte{0}, std::byte{0},
                          std::byte{0},   std::byte{0},   std::byte{0},   std::byte{0},   std::byte{0}, std::byte{0}};

    // Insert an offset value into the four bytes after the given index of the binary vector
    auto insertOffset = [&binary](const size_t i, const uint32_t offset) {
        binary[i] = static_cast<std::byte>(offset & 0xFF);
        binary[i + 1] = static_cast<std::byte>(offset >> 8 & 0xFF);
        binary[i + 2] = static_cast<std::byte>(offset >> 16 & 0xFF);
        binary[i + 3] = static_cast<std::byte>(offset >> 24 & 0xFF);
    };

    // Ensure length of binary vector is a multiple of 4
    auto padBinary = [&binary] {
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
        std::vector<std::byte> textBytes = layout.data.at(MemSection::TEXT);
        binary.insert(binary.end(), textBytes.begin(), textBytes.end());
        padBinary();
    }
    if (layout.data.contains(MemSection::DATA)) {
        insertOffset(8, binary.size());
        // Add size byte of section
        binary.insert(binary.end(), 4, std::byte{0});
        insertOffset(binary.size() - 4, layout.data.at(MemSection::DATA).size());
        // Add static data to binary
        std::vector<std::byte> dataBytes = layout.data.at(MemSection::DATA);
        binary.insert(binary.end(), dataBytes.begin(), dataBytes.end());
        padBinary();
    }
    if (layout.data.contains(MemSection::KTEXT)) {
        insertOffset(12, binary.size());
        // Add size byte of section
        binary.insert(binary.end(), 4, std::byte{0});
        insertOffset(binary.size() - 4, layout.data.at(MemSection::KTEXT).size());
        // Add ktext data to binary
        std::vector<std::byte> ktextBytes = layout.data.at(MemSection::KTEXT);
        binary.insert(binary.end(), ktextBytes.begin(), ktextBytes.end());
        padBinary();
    }
    if (layout.data.contains(MemSection::KDATA)) {
        insertOffset(16, binary.size());
        // Add size byte of section
        binary.insert(binary.end(), 4, std::byte{0});
        insertOffset(binary.size() - 4, layout.data.at(MemSection::KDATA).size());
        // Add kdata to binary
        std::vector<std::byte> kdataBytes = layout.data.at(MemSection::KDATA);
        binary.insert(binary.end(), kdataBytes.begin(), kdataBytes.end());
        padBinary();
    }
    if (debug && !layout.debugInfo.empty()) {
        const std::vector<std::byte> binaryDebugInfo = serializeDebugInfo(layout.debugInfo);
        insertOffset(20, binary.size());
        binary.insert(binary.end(), 4, std::byte{0});
        insertOffset(binary.size() - 4, binaryDebugInfo.size());
        // Add debug info to binary
        binary.insert(binary.end(), binaryDebugInfo.begin(), binaryDebugInfo.end());
        padBinary();
    }

    return binary;
}


MemLayout loadLayout(const std::vector<std::byte>& binary) {
    // Check if the binary starts with the MASM magic number
    if (binary.size() < 4 || binary[0] != std::byte{'M'} || binary[1] != std::byte{'A'} ||
        binary[2] != std::byte{'S'} || binary[3] != std::byte{'M'}) {
        throw std::runtime_error("Invalid MASM binary format");
    }

    auto extractOffset = [&binary](const size_t index) {
        return static_cast<uint32_t>(binary.at(index)) | static_cast<uint32_t>(binary.at(index + 1)) << 8 |
               static_cast<uint32_t>(binary.at(index + 2)) << 16 | static_cast<uint32_t>(binary.at(index + 3)) << 24;
    };

    const std::vector<size_t> secHeaders = {extractOffset(4), extractOffset(8), extractOffset(12), extractOffset(16),
                                            extractOffset(20)};
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
    if (secHeaders[4] > 0) {
        const size_t debugInfoSize = extractOffset(secHeaders[4]);
        std::vector<std::byte> binaryDebugInfo;
        for (size_t i = 0; i < debugInfoSize; i++)
            binaryDebugInfo.push_back(binary.at(secHeaders[4] + 4 + i));
        layout.debugInfo = deSerializeDebugInfo(binaryDebugInfo);
    }

    return layout;
}
