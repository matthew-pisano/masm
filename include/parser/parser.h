//
// Created by matthew on 4/14/25.
//

#ifndef PARSER_H
#define PARSER_H

#include <cstdint>

#include "interpreter/memory.h"
#include "labels.h"
#include "tokenizer/tokenizer.h"


/**
 * Class to parse a sequence of tokens into memory allocations ready for execution
 */
class Parser {

    /**
     * Whether to parse to a little endian memory layout
     */
    bool useLittleEndian;

    /**
     * Parses an instruction and its arguments into bytes that can be allocated to memory
     * @param loc The location in which the instruction will be placed into memory
     * @param instrToken The token containing the instruction
     * @param args The argument tokens for the instruction
     * @return The memory allocation associated with the instruction
     * @throw runtime_error When an instruction is malformed
     */
    std::vector<std::byte> parseInstruction(uint32_t loc, const Token& instrToken,
                                            std::vector<Token>& args);

    /**
     * Parses an R-type instruction into bytes that can be allocated to memory
     * @param rd The index of the rd register
     * @param rs The index of the rs register
     * @param rt The index of the rt register
     * @param shamt The shift amount for the instruction
     * @param funct The function code for the instruction
     * @return The memory allocation associated with the instruction
     */
    std::vector<std::byte> parseRTypeInstruction(uint32_t rd, uint32_t rs, uint32_t rt,
                                                 uint32_t shamt, uint32_t funct) const;

    /**
     * Parses an I-type instruction into bytes that can be allocated to memory
     * @param loc The location in which the instruction will be placed into memory
     * @param opcode The opcode for the instruction
     * @param rt The index of the rt register
     * @param rs The index of the rs register
     * @param immediate The immediate value for the instruction
     * @return The memory allocation associated with the instruction
     * @throw runtime_error When a branch target is out of range
     */
    std::vector<std::byte> parseITypeInstruction(uint32_t loc, uint32_t opcode, uint32_t rt,
                                                 uint32_t rs, int32_t immediate) const;

    /**
     * Parses a J-type instruction into bytes that can be allocated to memory
     * @param opcode The opcode for the instruction
     * @param address The address passed into the instruction
     * @return The memory allocation associated with the instruction
     */
    std::vector<std::byte> parseJTypeInstruction(uint32_t opcode, uint32_t address) const;

    /**
     * A specialized function to parse the syscall instruction
     * @return The memory allocation associated with the syscall instruction
     */
    std::vector<std::byte> parseSyscallInstruction() const;

    /**
     * A specialized function to parse the eret instruction
     * @return The memory allocation associated with the eret instruction
     */
    std::vector<std::byte> parseEretInstruction() const;

    /**
     * Parses a CP0 instruction into bytes that can be allocated to memory
     * @param op Stores operation of the instruction
     * @param rt The index of the rt register
     * @param rd The index of the rd register
     * @return The memory allocation associated with the CP0 move instruction
     */
    std::vector<std::byte> parseCP0Instruction(uint32_t op, uint32_t rt, uint32_t rd) const;

    /**
     * Parses a CP1 register type instruction into bytes that can be allocated to memory
     * @param fmt Stores the format of the instruction
     * @param ft Stores the index of the ft register
     * @param fs Stores the index of the fs register
     * @param fd Stores the index of the fd register
     * @param func Stores the function code of the instruction
     * @return The memory allocation associated with the CP1 instruction
     */
    std::vector<std::byte> parseCP1RegInstruction(uint32_t fmt, uint32_t ft, uint32_t fs,
                                                  uint32_t fd, uint32_t func) const;

    /**
     * Parses a CP1 register immediate type instruction into bytes that can be allocated to memory
     * @param sub Stores the sub-operation of the instruction
     * @param rt The index of the rt register
     * @param fs The index of the fs register
     * @return The memory allocation associated with the CP1 instruction
     */
    std::vector<std::byte> parseCP1RegImmInstruction(uint32_t sub, uint32_t rt, uint32_t fs) const;

    /**
     * Parses a CP1 immediate type instruction into bytes that can be allocated to memory
     * @param op Stores the operation of the instruction
     * @param base Stores the base register index
     * @param ft The index of the ft register
     * @param offset The immediate offset value for the instruction
     * @return The memory allocation associated with the CP1 instruction
     */
    std::vector<std::byte> parseCP1ImmInstruction(uint32_t op, uint32_t base, uint32_t ft,
                                                  uint32_t offset) const;

    /**
     * Parses a CP1 conditional instruction into bytes that can be allocated to memory
     * @param fmt Stores the format of the instruction
     * @param ft Stores the index of the ft register
     * @param fs Stores the index of the fs register
     * @param cond Stores the condition code for the instruction
     * @return The memory allocation associated with the CP1 instruction
     */
    std::vector<std::byte> parseCP1CondInstruction(uint32_t fmt, uint32_t ft, uint32_t fs,
                                                   uint32_t cond) const;

    /**
     * Parses a CP1 conditional immediate instruction into bytes that can be allocated to memory
     * @param loc The location in which the instruction will be placed into memory
     * @param tf Stores the true/false flag for the instruction
     * @param offset The immediate offset value for the instruction
     * @return The memory allocation associated with the CP1 instruction
     */
    std::vector<std::byte> parseCP1CondImmInstruction(uint32_t loc, uint32_t tf,
                                                      int32_t offset) const;

    /**
     * Replaces all pseudo instructions in the given lines with their concrete counterparts
     * @param tokens The lines of tokens to resolve pseudo instructions for
     * @throw runtime_error When an unknown pseudo instruction is passed
     */
    void resolvePseudoInstructions(std::vector<LineTokens>& tokens);

    /**
     * A helper method to parse the common formats of branch pseudo instructions
     * @param reg1 The first register token
     * @param reg2 The second register token
     * @param label The label address token for the branch
     * @param checkLt Whether the set less than instruction is used for this branch type
     * @param checkEq Whether the branch equal instruction is used for this branch type
     * @return The lines of tokens that represent the parsed branch pseudo instruction
     */
    std::vector<std::vector<Token>> parseBranchPseudoInstruction(const Token& reg1,
                                                                 const Token& reg2,
                                                                 const Token& label, bool checkLt,
                                                                 bool checkEq);

    /**
     * Parse a single line of tokens into memory allocations
     * @param layout The memory layout to populate
     * @param currSection The current section of memory being populated
     * @param tokenLine The line of tokens to parse
     * @throw runtime_error When an error is encountered during parsing
     */
    void parseLine(MemLayout& layout, MemSection& currSection, const LineTokens& tokenLine);

protected:
    /**
     * A class to manage the mapping of labels to memory locations
     */
    LabelMap labelMap;

public:
    /**
     * Constructor for the Parser class
     * @param useLittleEndian Whether to use little endian memory layout
     */
    explicit Parser(const bool useLittleEndian = false) : useLittleEndian(useLittleEndian) {}

    /**
     * Parses a sequence of tokens into memory allocations ready for execution
     * @param tokenLines The program tokens to parse
     * @return The memory allocations associated with the program
     * @throw MasmSyntaxError When an error is encountered during parsing
     */
    MemLayout parse(const std::vector<LineTokens>& tokenLines);


    /**
     * Fetches the label map associated with this parser
     * @return The label map associated with this parser
     */
    LabelMap& getLabels();
};

#endif // PARSER_H
