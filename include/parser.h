//
// Created by matthew on 4/14/25.
//

#ifndef PARSER_H
#define PARSER_H
#include <cstdint>
#include <map>

#include "memory.h"
#include "tokenizer.h"


/**
 * Class to parse a sequence of tokens into memory allocations ready for execution
 */
class Parser {

    /**
     * A map between the names of labels and their associated memory addresses
     */
    std::map<std::string, uint32_t> labelMap;

    /**
     * Modifies instruction arguments to replace label references with labeled memory locations
     * @param instructionArgs The instruction arguments to modify
     * @throw runtime_error When one of the arguments references an unknown label
     */
    void resolveLabels(std::vector<Token>& instructionArgs);

    /**
     * Populates the label map prior to processing using static allocations for the given tokens
     * @param tokens The program tokens
     * @throw runtime_error When a duplicate label definition is detected
     */
    void populateLabelMap(const std::vector<std::vector<Token>>& tokens);

    /**
     * Parses a directive and its arguments into bytes that can be allocated to memory
     * @param dirToken The token for the directive
     * @param args Any argument tokens to pass to the directive
     * @return The memory allocation associated with the directive
     * @throw runtime_error When the arguments for a directive are malformed
     */
    static std::vector<std::byte> parseDirective(const Token& dirToken,
                                                 const std::vector<Token>& args);

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
    static std::vector<std::byte> parseRTypeInstruction(uint32_t rd, uint32_t rs, uint32_t rt,
                                                        uint32_t shamt, uint32_t funct);

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
    static std::vector<std::byte> parseITypeInstruction(uint32_t loc, uint32_t opcode, uint32_t rt,
                                                        uint32_t rs, int32_t immediate);

    /**
     * Parses a J-type instruction into bytes that can be allocated to memory
     * @param opcode The opcode for the instruction
     * @param address The address passed into the instruction
     * @return The memory allocation associated with the instruction
     */
    static std::vector<std::byte> parseJTypeInstruction(uint32_t opcode, uint32_t address);

    /**
     * A specialized function to parse the syscall instruction
     * @return The memory allocation associated with the syscall instruction
     */
    static std::vector<std::byte> parseSyscallInstruction();

    /**
     * A more generalized function to parse pseudo instructions
     * @param loc The location in which the instruction will be placed into memory
     * @param instructionName The name of the pseudo instruction
     * @param args The argument tokens for the pseudo instruction
     * @return The memory allocation associated with the pseudo instruction
     * @throw runtime_error When an unknown pseudo instruction is passed
     */
    std::vector<std::byte> parsePseudoInstruction(uint32_t loc, const std::string& instructionName,
                                                  const std::vector<Token>& args);

    /**
     * A helper method to parse the common formats of branch pseudo instructions
     * @param loc The location in which the instruction will be placed into memory
     * @param reg1 The first register token
     * @param reg2 The second register token
     * @param label The label token for the branch
     * @param checkLt Whether the set less than instruction is used for this branch type
     * @param checkEq Whether the branch equal instruction is used for this branch type
     * @return The memory allocation associated with the branch pseudo instruction
     */
    std::vector<std::byte> parseBranchPseudoInstruction(uint32_t loc, const Token& reg1,
                                                        const Token& reg2, const Token& label,
                                                        bool checkLt, bool checkEq);

    /**
     * Parse a single line of tokens into memory allocations
     * @param layout The memory layout to populate
     * @param currSection The current section of memory being populated
     * @param tokenLine The line of tokens to parse
     */
    void parseLine(MemLayout& layout, MemSection& currSection, const std::vector<Token>& tokenLine);

public:
    /**
     * Parses a sequence of tokens into memory allocations ready for execution
     * @param tokens The program tokens to parse
     * @return The memory allocations associated with the program
     * @throw runtime_error When an error is encountered during parsing
     */
    MemLayout parse(const std::vector<std::vector<Token>>& tokens);
};

#endif // PARSER_H
