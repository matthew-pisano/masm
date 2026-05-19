//
// Created by matthew on 6/7/25.
//

#include "interpreter/cp0.h"

#include <stdexcept>

#include "parser/instruction.h"


// Coprocessor 0 register access
int32_t Coproc0RegisterFile::operator[](const uint32_t index) const { return registers.at(index); }
int32_t& Coproc0RegisterFile::operator[](const uint32_t index) { return registers.at(index); }

int32_t Coproc0RegisterFile::operator[](const Coproc0Register index) const {
    return registers.at(static_cast<uint32_t>(index));
}
int32_t& Coproc0RegisterFile::operator[](const Coproc0Register index) {
    return registers.at(static_cast<uint32_t>(index));
}


void execCP0Type(Coproc0RegisterFile& cp0, RegisterFile& registers, const uint32_t rs,
                 const uint32_t rt, const uint32_t rd) {
    switch (static_cast<InstructionCode>(rs)) {
        case InstructionCode::MFC0: {
            // Move from CP0
            registers[rt] = cp0[rd];
            break;
        }
        case InstructionCode::MTC0: {
            // Move to CP0
            cp0[rd] = registers[rt];
            break;
        }
            // Should never be reached
        default:
            throw std::runtime_error("Unknown Co-Processor 0 instruction " + std::to_string(rs));
    }
}


void execEret(Coproc0RegisterFile& cp0, RegisterFile& registers) {
    // Restore the PC from the saved address in the EPC register
    registers[Register::PC] = cp0[Coproc0Register::EPC];
    cp0[Coproc0Register::EPC] = 0;

    // Clear the cause register
    cp0[Coproc0Register::CAUSE] = 0;
}
