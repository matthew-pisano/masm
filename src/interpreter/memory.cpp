//
// Created by matthew on 4/16/25.
//

#include "interpreter/memory.h"

#include <stdexcept>

#include "exceptions.h"
#include "utils.h"


std::byte Memory::_sysByteAt(const uint32_t index) const {
    if (!memory.contains(index))
        // Default of zero if not found
        return static_cast<std::byte>(0);
    return memory.at(index);
}


int32_t Memory::_sysWordAt(const uint32_t index) const {
    if (useLittleEndian)
        // If little-endian, read bytes in little endian order
        return static_cast<int32_t>(_sysByteAt(index + 3)) << 24 |
               static_cast<int32_t>(_sysByteAt(index + 2)) << 16 |
               static_cast<int32_t>(_sysByteAt(index + 1)) << 8 |
               static_cast<int32_t>(_sysByteAt(index));

    // If big-endian, read bytes in big endian order
    return static_cast<int32_t>(_sysByteAt(index)) << 24 |
           static_cast<int32_t>(_sysByteAt(index + 1)) << 16 |
           static_cast<int32_t>(_sysByteAt(index + 2)) << 8 |
           static_cast<int32_t>(_sysByteAt(index + 3));
}


void Memory::_sysWordTo(const uint32_t index, const int32_t value) {
    if (useLittleEndian) {
        // If little-endian, write bytes in little endian order
        memory[index] = static_cast<std::byte>(value);
        memory[index + 1] = static_cast<std::byte>(value >> 8);
        memory[index + 2] = static_cast<std::byte>(value >> 16);
        memory[index + 3] = static_cast<std::byte>(value >> 24);
    } else {
        memory[index] = static_cast<std::byte>(value >> 24);
        memory[index + 1] = static_cast<std::byte>(value >> 16);
        memory[index + 2] = static_cast<std::byte>(value >> 8);
        memory[index + 3] = static_cast<std::byte>(value);
    }
}


void Memory::readSideEffect(const uint32_t index) {
    const uint32_t input_ready = memSectionOffset(MemSection::MMIO);
    const uint32_t input_data = input_ready + 4;

    // Check if reading from input data word
    if (index >= input_data && index < input_data + 4)
        // Reset input ready bit
        _sysWordTo(input_ready, 0);
}


void Memory::writeSideEffect(const uint32_t index) {
    const uint32_t input_ready = memSectionOffset(MemSection::MMIO);
    const uint32_t input_data = input_ready + 4;
    const uint32_t output_ready = input_data + 4;
    const uint32_t output_data = output_ready + 4;

    // Check if writing to input or output ready bits or input data word
    if ((index >= output_ready && index < output_ready + 4) ||
        (index >= input_ready && index < input_ready + 4) ||
        (index >= input_data && index < input_data + 4))
        throw std::runtime_error("Invalid write into read-only memory at " + hex_to_string(index));

    // Check if writing to output data word
    if (index >= output_data && index < output_data + 4)
        // Reset output ready bit
        _sysWordTo(output_ready, 0);
}


int32_t Memory::wordAt(const uint32_t index) {
    if (index % 4 != 0)
        throw ExecExcept("Invalid word access at " + hex_to_string(index),
                         EXCEPT_CODE::ADDRESS_EXCEPTION_LOAD);

    readSideEffect(index);
    return _sysWordAt(index);
}


uint16_t Memory::halfAt(const uint32_t index) {
    if (index % 2 != 0)
        throw ExecExcept("Invalid half-word access at " + hex_to_string(index),
                         EXCEPT_CODE::ADDRESS_EXCEPTION_LOAD);

    readSideEffect(index);
    if (useLittleEndian)
        // If little-endian, read bytes in little endian order
        return static_cast<uint16_t>(_sysByteAt(index + 1)) << 8 |
               static_cast<uint16_t>(_sysByteAt(index));

    return static_cast<uint16_t>(_sysByteAt(index)) << 8 |
           static_cast<uint16_t>(_sysByteAt(index + 1));
}


uint8_t Memory::byteAt(const uint32_t index) {
    readSideEffect(index);
    return static_cast<uint8_t>(_sysByteAt(index));
}


void Memory::wordTo(const uint32_t index, const int32_t value) {
    if (index % 4 != 0)
        throw ExecExcept("Invalid word access at " + hex_to_string(index),
                         EXCEPT_CODE::ADDRESS_EXCEPTION_STORE);

    writeSideEffect(index);
    _sysWordTo(index, value);
}


void Memory::halfTo(const uint32_t index, const int16_t value) {
    if (index % 2 != 0)
        throw ExecExcept("Invalid half-word access at " + hex_to_string(index),
                         EXCEPT_CODE::ADDRESS_EXCEPTION_STORE);

    writeSideEffect(index);
    if (useLittleEndian) {
        // If little-endian, write bytes in little endian order
        memory[index] = static_cast<std::byte>(value);
        memory[index + 1] = static_cast<std::byte>(value >> 8);
    } else {
        // If big-endian, write bytes in big endian order
        memory[index] = static_cast<std::byte>(value >> 8);
        memory[index + 1] = static_cast<std::byte>(value);
    }
}


void Memory::byteTo(const uint32_t index, const int8_t value) {
    writeSideEffect(index);
    memory[index] = static_cast<std::byte>(value);
}

bool Memory::isValid(const uint32_t index) const { return memory.contains(index); }


std::byte Memory::operator[](const uint32_t index) const { return memory.at(index); }
std::byte& Memory::operator[](const uint32_t index) { return memory[index]; }


MemSection nameToMemSection(const std::string& name) {
    if (name == "data")
        return MemSection::DATA;
    if (name == "text")
        return MemSection::TEXT;
    if (name == "ktext")
        return MemSection::KTEXT;
    if (name == "kdata")
        return MemSection::KDATA;
    // Should never be reached
    throw std::runtime_error("Unknown memory directive " + name);
}


uint32_t memSectionOffset(const MemSection section) {
    switch (section) {
        case MemSection::DATA:
            return 0x10010000;
        case MemSection::HEAP:
            return 0x10040000; // Heap grows upwards, so this is the start of the heap
        case MemSection::GLOBAL:
            return 0x10008000;
        case MemSection::STACK:
            return 0x7fffeffc; // Stack grows downwards, so this is the top of the stack
        case MemSection::TEXT:
            return 0x00400000;
        case MemSection::KDATA:
            return 0x90000000;
        case MemSection::KTEXT:
            return 0x80000000;
        case MemSection::MMIO:
            return 0xffff0000;
    }
    // Should never be reached
    throw std::runtime_error("Unknown memory section " + std::to_string(static_cast<int>(section)));
}


bool isSectionExecutable(const MemSection section) {
    return section == MemSection::TEXT || section == MemSection::KTEXT;
}
