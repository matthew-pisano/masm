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
 * A type alias for an object containing memory allocations from the parser
 */
using MemLayout = std::map<MemSection, std::vector<uint8_t>>;


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
     */
    void resolveLabels(std::vector<Token>& instructionArgs);

    /**
     * Populates the label map prior to processing using static allocations for the given tokens
     * @param tokens The program tokens
     */
    void populateLabels(const std::vector<std::vector<Token>>& tokens);

    /**
     * Parses a directive and its arguments into bytes that can be allocated to memory
     * @param dirToken The token for the directive
     * @param args Any argument tokens to pass to the directive
     * @return The memory allocation associated with the directive
     */
    static std::vector<uint8_t> parseDirective(const Token& dirToken,
                                               const std::vector<Token>& args);

    /**
     * Parses an instruction and its arguments into bytes that can be allocated to memory
     * @param loc The location in which the instruction will be placed into memory
     * @param instrToken The token containing the instruction
     * @param args The argument tokens for the instruction
     * @return The memory allocation associated with the instruction
     */
    std::vector<uint8_t> parseInstruction(uint32_t loc, const Token& instrToken,
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
    static std::vector<uint8_t> parseRTypeInstruction(uint32_t rd, uint32_t rs, uint32_t rt,
                                                      uint32_t shamt, uint32_t funct);

    /**
     * Parses an I-type instruction into bytes that can be allocated to memory
     * @param loc The location in which the instruction will be placed into memory
     * @param opcode The opcode for the instruction
     * @param rt The index of the rt register
     * @param rs The index of the rs register
     * @param immediate The immediate value for the instruction
     * @return The memory allocation associated with the instruction
     */
    static std::vector<uint8_t> parseITypeInstruction(uint32_t loc, uint32_t opcode, uint32_t rt,
                                                      uint32_t rs, int32_t immediate);

    /**
     * Parses a J-type instruction into bytes that can be allocated to memory
     * @param opcode The opcode for the instruction
     * @param address The address passed into the instruction
     * @return The memory allocation associated with the instruction
     */
    static std::vector<uint8_t> parseJTypeInstruction(uint32_t opcode, uint32_t address);

    /**
     * A specialized function to parse the syscall instruction
     * @return The memory allocation associated with the syscall instruction
     */
    static std::vector<uint8_t> parseSyscallInstruction();

    /**
     * A more generalized function to parse pseudo instructions
     * @param loc The location in which the instruction will be placed into memory
     * @param instructionName The name of the pseudo instruction
     * @param args The argument tokens for the pseudo instruction
     * @return The memory allocation associated with the pseudo instruction
     */
    std::vector<uint8_t> parsePseudoInstruction(uint32_t loc, const std::string& instructionName,
                                                std::vector<Token>& args);

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
    std::vector<uint8_t> parseBranchPseudoInstruction(uint32_t loc, const Token& reg1,
                                                      const Token& reg2, const Token& label,
                                                      bool checkLt, bool checkEq);

public:
    /**
     * Parses a sequence of tokens into memory allocations ready for execution
     * @param tokens The program tokens to parse
     * @return The memory allocations associated with the program
     */
    MemLayout parse(const std::vector<std::vector<Token>>& tokens);
};

#endif // PARSER_H
