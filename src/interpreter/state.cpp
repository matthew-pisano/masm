//
// Created by matthew on 6/8/25.
//

#include "interpreter/state.h"

#include "exceptions.h"


std::string causeToString(const uint32_t cause) {
    if (cause & static_cast<uint32_t>(INTERP_CODE::KEYBOARD_INTERP))
        return "MMIO read interrupt failed";
    if (cause & static_cast<uint32_t>(INTERP_CODE::DISPLAY_INTERP))
        return "MMIO write interrupt failed";

    const uint32_t excCode = cause & 0x007c; // Zero out all bits except for [2-6]
    if (excCode == static_cast<uint32_t>(EXCEPT_CODE::ADDRESS_EXCEPTION_LOAD))
        return "Failed to load address";
    if (excCode == static_cast<uint32_t>(EXCEPT_CODE::ADDRESS_EXCEPTION_STORE))
        return "Failed to store address";
    if (excCode == static_cast<uint32_t>(EXCEPT_CODE::SYSCALL_EXCEPTION))
        return "Failed to execute syscall";
    if (excCode == static_cast<uint32_t>(EXCEPT_CODE::BREAKPOINT_EXCEPTION))
        return "Failed to handle breakpoint";
    if (excCode == static_cast<uint32_t>(EXCEPT_CODE::RESERVED_INSTRUCTION_EXCEPTION))
        return "Attempted to execute reserved instruction";
    if (excCode == static_cast<uint32_t>(EXCEPT_CODE::ARITHMETIC_OVERFLOW_EXCEPTION))
        return "Integer overflow";
    if (excCode == static_cast<uint32_t>(EXCEPT_CODE::TRAP_EXCEPTION))
        return "Trap exception occurred";
    if (excCode == static_cast<uint32_t>(EXCEPT_CODE::DIVIDE_BY_ZERO_EXCEPTION))
        return "Division by zero";
    if (excCode == static_cast<uint32_t>(EXCEPT_CODE::FLOATING_POINT_OVERFLOW))
        return "Floating point overflow";
    if (excCode == static_cast<uint32_t>(EXCEPT_CODE::FLOATING_POINT_UNDERFLOW))
        return "Floating point underflow";

    return "Unknown exception code: " + std::to_string(excCode);
}


SourceLocator State::getDebugInfo(const uint32_t addr) const { return *debugInfo.at(addr); }


void State::loadProgram(const MemLayout& layout) {
    for (const std::pair<MemSection, std::vector<std::byte>> pair : layout.data)
        for (size_t i = 0; i < pair.second.size(); i++) {
            uint32_t memOffset = memSectionOffset(pair.first) + i;
            memory[memOffset] = pair.second[i];

            // Add debug info from layout if the address is executable text
            if (isSectionExecutable(pair.first)) {
                const std::shared_ptr<SourceLocator> sourceLine =
                        layout.debugInfo.at(pair.first).at(i);
                debugInfo[memOffset] = sourceLine;
            }
        }
}
