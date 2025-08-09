//
// Created by matthew on 4/14/25.
//

#include "parser/parser.h"

#include <algorithm>
#include <stdexcept>

#include "exceptions.h"
#include "interpreter/cp1.h"
#include "interpreter/cpu.h"
#include "parser/directive.h"
#include "parser/instruction.h"
#include "tokenizer/postprocessor.h"
#include "utils.h"


LabelMap& Parser::getLabels() { return labelMap; }


MemLayout Parser::parse(const std::vector<LineTokens>& tokenLines, const bool raw) {
    MemLayout layout;
    std::vector<LineTokens> modifiedTokenLines = tokenLines;

    MemSection currSection = MemSection::TEXT;
    layout.data[MemSection::TEXT] = {};

    if (!raw) {
        // Insert dummy nop instruction to save space in case a jump main is needed
        const std::vector<Token> nop = {{TokenCategory::INSTRUCTION, "nop"}};
        modifiedTokenLines.insert(modifiedTokenLines.begin(), {"<unknown>", 0, nop});
    }

    // Resolve all labels before parsing instructions
    labelMap.populateLabelMap(modifiedTokenLines);

    // Insert jump to main instruction if the label is defined, otherwise start at first text word
    if (!raw && labelMap.contains("main")) {
        const std::vector<Token> jumpMain = {{TokenCategory::INSTRUCTION, "j"},
                                             {TokenCategory::LABEL_REF, "main"}};
        modifiedTokenLines[0] = {"<unknown>", 0, jumpMain};
    }

    // Resolve pseudo instructions in the token lines
    resolvePseudoInstructions(modifiedTokenLines);

    for (const auto& tokenLine : modifiedTokenLines) {
        // Skip empty lines
        if (tokenLine.tokens.empty())
            continue;

        try {
            parseLine(layout, currSection, tokenLine);
        } catch (const std::runtime_error& e) {
            throw MasmSyntaxError(e.what(), tokenLine.filename, tokenLine.lineno);
        }
    }

    return layout;
}


void Parser::parseLine(MemLayout& layout, MemSection& currSection, const LineTokens& tokenLine) {
    // Get next open location in memory
    const uint32_t memLoc = memSectionOffset(currSection) + layout.data[currSection].size();

    const Token& firstToken = tokenLine.tokens[0];
    const std::vector unfilteredArgs(tokenLine.tokens.begin() + 1, tokenLine.tokens.end());
    std::vector<Token> args = filterTokenList(unfilteredArgs);

    DebugInfo debugInfo;
    std::vector<std::byte> memBytes;
    switch (firstToken.category) {
        case TokenCategory::SEC_DIRECTIVE: {
            currSection = nameToMemSection(firstToken.value);
            // If the section does not exist in the layout, create it
            if (!layout.data.contains(currSection))
                layout.data[currSection] = {};
            break;
        }
        case TokenCategory::ALLOC_DIRECTIVE: {
            // Resolve label references to their integer values before parsing
            labelMap.resolveLabels(args);

            const std::tuple<std::vector<std::byte>, size_t> alloc =
                    parsePaddedAllocDirective(memLoc, firstToken, args, useLittleEndian);
            memBytes.insert(memBytes.end(), std::get<0>(alloc).begin(), std::get<0>(alloc).end());
            const size_t paddedMemLoc = memLoc + std::get<1>(alloc);
            debugInfo.source = {tokenLine.filename, tokenLine.lineno, ""};
            try {
                debugInfo.label = labelMap.lookupLabel(paddedMemLoc);
            } catch (const std::runtime_error&) {
            }
            layout.debugInfo[paddedMemLoc] = debugInfo;
            break;
        }
        case TokenCategory::INSTRUCTION: {
            const std::vector<std::byte> instrBytes = parseInstruction(memLoc, firstToken, args);
            memBytes.insert(memBytes.end(), instrBytes.begin(), instrBytes.end());
            debugInfo.source = {tokenLine.filename, tokenLine.lineno, ""};
            try {
                debugInfo.label = labelMap.lookupLabel(memLoc);
            } catch (const std::runtime_error&) {
            }

            // Add the source code text the debug info
            for (const Token& token : tokenLine.tokens) {
                if (token.category == TokenCategory::SEPERATOR)
                    debugInfo.source.text += token.value;
                else if (token.category == TokenCategory::REGISTER)
                    debugInfo.source.text += " $" + token.value;
                else if (token.category != TokenCategory::LABEL_REF)
                    debugInfo.source.text += " " + token.value;
                else
                    debugInfo.source.text += " " + unmangleLabel(token.value);

                if (debugInfo.source.text[0] == ' ')
                    debugInfo.source.text.erase(0, 1); // Remove leading space
            }

            // Assign debug info to all allocated instructions (including multi-instruction
            // pseudo-instructions)
            for (size_t i = 0; i < instrBytes.size(); i += 4) {
                layout.debugInfo[memLoc + i] = debugInfo;
                // Only label the first instruction in a pseudo-instruction
                if (i > 0)
                    layout.debugInfo[memLoc + i].label = "";
            }
            break;
        }
        case TokenCategory::LABEL_DEF:
            break; // Ignore label definitions, they do not add any additional memory allocations
        default: {
            if (firstToken.category != TokenCategory::LABEL_REF)
                throw std::runtime_error("Encountered unexpected token '" + firstToken.value + "'");
            throw std::runtime_error("Encountered unexpected token '" +
                                     unmangleLabel(firstToken.value) + "'");
        }
    }

    if (memBytes.empty())
        return;

    layout.data[currSection].insert(layout.data[currSection].end(), memBytes.begin(),
                                    memBytes.end());
}


std::vector<std::byte> Parser::parseInstruction(const uint32_t loc, const Token& instrToken,
                                                std::vector<Token>& args) {

    // Throw error if pattern for instruction is invalid
    validateInstruction(instrToken, args);

    // Resolve label references to their computed address values
    labelMap.resolveLabels(args);

    InstructionOp instructionOp = nameToInstructionOp(instrToken.value);
    std::vector<uint32_t> argCodes = {};
    // Parse the instruction argument token values into integers
    for (const Token& arg : args) {
        switch (arg.category) {
            case TokenCategory::IMMEDIATE:
                argCodes.push_back(stoui32(arg.value));
                break;
            case TokenCategory::REGISTER: {
                if (isSignedInteger(arg.value))
                    // If the register is an integer, use it as the register index
                    argCodes.push_back(stoui32(arg.value));
                else {
                    // Otherwise, use the register name to get the index
                    if (arg.value.starts_with("f"))
                        argCodes.push_back(Coproc1RegisterFile::indexFromName(arg.value));
                    else
                        argCodes.push_back(RegisterFile::indexFromName(arg.value));
                }
                break;
            }
            default:
                // Should never be reached
                throw std::runtime_error("Invalid argument type " +
                                         std::to_string(static_cast<int>(arg.category)));
        }
    }

    const uint32_t opFuncCode = static_cast<uint32_t>(instructionOp.opFuncCode);
    // Parse integer arguments into a single instruction word
    switch (instructionOp.type) {
        // Core CPU Instructions
        case InstructionType::R_TYPE_D_S_T:
            return parseRTypeInstruction(argCodes[0], argCodes[1], argCodes[2], 0x00, opFuncCode);
        case InstructionType::R_TYPE_D_T_S:
            return parseRTypeInstruction(argCodes[0], argCodes[2], argCodes[1], 0x00, opFuncCode);
        case InstructionType::R_TYPE_D_T_H:
            return parseRTypeInstruction(argCodes[0], 0x00, argCodes[1], argCodes[2], opFuncCode);
        case InstructionType::R_TYPE_D:
            return parseRTypeInstruction(argCodes[0], 0x00, 0x00, 0x00, opFuncCode);
        case InstructionType::R_TYPE_S:
            return parseRTypeInstruction(0x00, argCodes[0], 0x00, 0x00, opFuncCode);
        case InstructionType::I_TYPE_T_S_I:
            return parseITypeInstruction(loc, opFuncCode, argCodes[0], argCodes[1],
                                         static_cast<int32_t>(argCodes[2]));
        case InstructionType::I_TYPE_S_T_L:
            // Instructions where rs comes before rt in the binary encoding
            return parseITypeInstruction(loc, opFuncCode, argCodes[1], argCodes[0],
                                         static_cast<int32_t>(argCodes[2]));
        case InstructionType::I_TYPE_T_I:
            // Location not needed for short I-Type instructions
            return parseITypeInstruction(0x00, opFuncCode, argCodes[0], 0x00,
                                         static_cast<int32_t>(argCodes[1]));
        case InstructionType::R_TYPE_S_T:
            return parseRTypeInstruction(0x00, argCodes[0], argCodes[1], 0x00, opFuncCode);
        case InstructionType::J_TYPE_L:
            return parseJTypeInstruction(opFuncCode, argCodes[0]);
        case InstructionType::SYSCALL:
            return parseSyscallInstruction();

        // Co-Processor 0 Instructions
        case InstructionType::CP0_TYPE_T_D:
            return parseCP0Instruction(opFuncCode, argCodes[0], argCodes[1]);
        case InstructionType::ERET:
            return parseEretInstruction();

        // Co-Processor 1 Instructions (Floating Point)
        case InstructionType::CP1_TYPE_SP_D_S:
            return parseCP1RegInstruction(0x10, 0x00, argCodes[1], argCodes[0], opFuncCode);
        case InstructionType::CP1_TYPE_DP_D_S:
            return parseCP1RegInstruction(0x11, 0x00, argCodes[1], argCodes[0], opFuncCode);
        case InstructionType::CP1_TYPE_SP_D_S_T:
            return parseCP1RegInstruction(0x10, argCodes[2], argCodes[1], argCodes[0], opFuncCode);
        case InstructionType::CP1_TYPE_DP_D_S_T:
            return parseCP1RegInstruction(0x11, argCodes[2], argCodes[1], argCodes[0], opFuncCode);
        case InstructionType::CP1_TYPE_L:
            return parseCP1CondImmInstruction(loc, opFuncCode, static_cast<int32_t>(argCodes[0]));
        case InstructionType::CP1_TYPE_SP_S_T_C:
            return parseCP1CondInstruction(0x10, argCodes[1], argCodes[0], opFuncCode);
        case InstructionType::CP1_TYPE_DP_S_T_C:
            return parseCP1CondInstruction(0x11, argCodes[1], argCodes[0], opFuncCode);
        case InstructionType::CP1_TYPE_T_S:
            return parseCP1RegImmInstruction(opFuncCode, argCodes[0], argCodes[1]);
        case InstructionType::CP1_TYPE_T_S_I:
            return parseCP1ImmInstruction(opFuncCode, argCodes[1], argCodes[0], argCodes[2]);
        default:
            // Should never be reached
            throw std::runtime_error("Unknown instruction type " +
                                     std::to_string(static_cast<int>(instructionOp.type)));
    }
}


std::vector<std::byte> Parser::parseRTypeInstruction(const uint32_t rd, const uint32_t rs,
                                                     const uint32_t rt, const uint32_t shamt,
                                                     const uint32_t funct) const {

    // Combine fields into 32-bit instruction code
    const uint32_t instruction = (0 & 0x3F) << 26 | (rs & 0x1F) << 21 | (rt & 0x1F) << 16 |
                                 (rd & 0x1F) << 11 | (shamt & 0x1F) << 6 | funct & 0x3F;
    return useLittleEndian ? i32ToLEByte(instruction) : i32ToBEByte(instruction);
}


std::vector<std::byte> Parser::parseITypeInstruction(const uint32_t loc, const uint32_t opcode,
                                                     const uint32_t rt, const uint32_t rs,
                                                     int32_t immediate) const {

    // Modify immediate values to be relative to the location of the current instruction
    std::vector branchOpCodes = {static_cast<uint32_t>(InstructionCode::BEQ),
                                 static_cast<uint32_t>(InstructionCode::BNE)};
    if (std::ranges::find(branchOpCodes, opcode) != branchOpCodes.end()) {
        // Branch targets are always word-aligned, so divide by 4
        const int32_t pcOffset = (immediate - static_cast<int32_t>(loc) - 4) >> 2;
        if (pcOffset < -32768 || pcOffset > 32767)
            throw std::runtime_error("Branch instruction offset out of range");
        immediate = static_cast<int32_t>(pcOffset);
    }

    // Combine fields into 32-bit instruction code
    const uint32_t instruction =
            (opcode & 0x3F) << 26 | (rs & 0x1F) << 21 | (rt & 0x1F) << 16 | immediate & 0xFFFF;
    return useLittleEndian ? i32ToLEByte(instruction) : i32ToBEByte(instruction);
}


std::vector<std::byte> Parser::parseJTypeInstruction(const uint32_t opcode,
                                                     const uint32_t address) const {

    // Combine fields into 32-bit instruction code (address shifted by 2 to byte align to 4)
    const uint32_t instruction = (opcode & 0x3F) << 26 | (address & 0x3FFFFFF) >> 2;
    return useLittleEndian ? i32ToLEByte(instruction) : i32ToBEByte(instruction);
}


std::vector<std::byte> Parser::parseSyscallInstruction() const {
    return useLittleEndian ? i32ToLEByte(0x0000000C) : i32ToBEByte(0x0000000C);
}


std::vector<std::byte> Parser::parseEretInstruction() const {
    return useLittleEndian ? i32ToLEByte(0x42000018) : i32ToBEByte(0x42000018);
}


std::vector<std::byte> Parser::parseCP0Instruction(const uint32_t op, const uint32_t rt,
                                                   const uint32_t rd) const {
    // Combine fields into 32-bit instruction code
    const uint32_t instruction = (0x10 & 0x3F) << 26 | (op & 0x1F) << 21 | (rt & 0x1F) << 16 |
                                 (rd & 0x1F) << 11 | 0x00 & 0x7FF;
    return useLittleEndian ? i32ToLEByte(instruction) : i32ToBEByte(instruction);
}


std::vector<std::byte> Parser::parseCP1RegInstruction(const uint32_t fmt, const uint32_t ft,
                                                      const uint32_t fs, const uint32_t fd,
                                                      const uint32_t func) const {
    // Combine fields into 32-bit instruction code
    const uint32_t instruction = (0x11 & 0x3F) << 26 | (fmt & 0x1F) << 21 | (ft & 0x1F) << 16 |
                                 (fs & 0x1F) << 11 | (fd & 0x1F) << 6 | func & 0x3F;
    return useLittleEndian ? i32ToLEByte(instruction) : i32ToBEByte(instruction);
}


std::vector<std::byte> Parser::parseCP1RegImmInstruction(const uint32_t sub, const uint32_t rt,
                                                         const uint32_t fs) const {
    // Combine fields into 32-bit instruction code
    const uint32_t instruction = (0x11 & 0x3F) << 26 | (sub & 0x1F) << 21 | (rt & 0x1F) << 16 |
                                 (fs & 0x1F) << 11 | 0x00 & 0x7FF;
    return useLittleEndian ? i32ToLEByte(instruction) : i32ToBEByte(instruction);
}


std::vector<std::byte> Parser::parseCP1ImmInstruction(const uint32_t op, const uint32_t base,
                                                      const uint32_t ft,
                                                      const uint32_t offset) const {
    // Combine fields into 32-bit instruction code
    const uint32_t instruction =
            (op & 0x3F) << 26 | (base & 0x1F) << 21 | (ft & 0x1F) << 16 | offset & 0xFFFF;
    return useLittleEndian ? i32ToLEByte(instruction) : i32ToBEByte(instruction);
}

std::vector<std::byte> Parser::parseCP1CondInstruction(const uint32_t fmt, const uint32_t ft,
                                                       const uint32_t fs,
                                                       const uint32_t cond) const {
    // Combine fields into 32-bit instruction code
    const uint32_t instruction = (0x11 & 0x3F) << 26 | (fmt & 0x1F) << 21 | (ft & 0x1F) << 16 |
                                 (fs & 0x1F) << 11 | (0x00 & 0x07) << 8 | (0x00 & 0x03) << 6 |
                                 (0x03 & 0x03) << 4 | cond & 0xF;
    return useLittleEndian ? i32ToLEByte(instruction) : i32ToBEByte(instruction);
}

std::vector<std::byte> Parser::parseCP1CondImmInstruction(const uint32_t loc, const uint32_t tf,
                                                          int32_t offset) const {

    // Branch targets are always word-aligned, so divide by 4
    const int32_t pcOffset = (offset - static_cast<int32_t>(loc) - 4) >> 2;
    if (pcOffset < -32768 || pcOffset > 32767)
        throw std::runtime_error("Branch instruction offset out of range");
    offset = static_cast<int32_t>(pcOffset);

    // Combine fields into 32-bit instruction code
    const uint32_t instruction = (0x11 & 0x3F) << 26 | (0x08 & 0x1F) << 21 | (0x00 & 0x07) << 18 |
                                 (0x00 & 0x01) << 17 | (tf & 0x01) << 16 | offset & 0xFFFF;
    return useLittleEndian ? i32ToLEByte(instruction) : i32ToBEByte(instruction);
}


void Parser::resolvePseudoInstructions(std::vector<LineTokens>& tokens) {
    auto it = tokens.begin();
    while (it != tokens.end()) {
        LineTokens& tokenLine = *it;
        // Copy tokenLine since it will be updated along with the changing iterator
        LineTokens originalTokenLine = tokenLine;
        try {
            const Token& firstToken = tokenLine.tokens[0];
            if (firstToken.category != TokenCategory::INSTRUCTION ||
                nameToInstructionOp(firstToken.value).type != InstructionType::PSEUDO) {
                // If the first token is not an instruction, continue to the next line
                ++it;
                continue;
            }
            const std::string instructionName = firstToken.value;
            const std::vector unfilteredArgs(tokenLine.tokens.begin() + 1, tokenLine.tokens.end());
            std::vector<Token> args = filterTokenList(unfilteredArgs);

            validatePseudoInstruction(firstToken, args);

            // li $tx, imm -> addiu $tx, $zero, imm
            if (instructionName == "li") {
                tokenLine.tokens = {
                        {TokenCategory::INSTRUCTION, "addiu"}, args[0],
                        {TokenCategory::SEPERATOR, ","},       {TokenCategory::REGISTER, "zero"},
                        {TokenCategory::SEPERATOR, ","},       args[1]};
            }
            // la $tx, label -> lui $at, upperAddr; ori $tx, $at, lowerAddr
            else if (instructionName == "la") {
                uint32_t value;
                if (args[1].category == TokenCategory::LABEL_REF) {
                    if (!labelMap.contains(args[1].value))
                        throw std::runtime_error("Unknown label '" + unmangleLabel(args[1].value) +
                                                 "'");
                    value = labelMap[args[1].value];
                } else
                    value = stoui32(args[1].value);

                const unsigned int upperBytes = (value & 0xFFFF0000) >> 16;
                const unsigned int lowerBytes = value & 0x0000FFFF;

                tokenLine.tokens = {{TokenCategory::INSTRUCTION, "lui"},
                                    {TokenCategory::REGISTER, "at"},
                                    {TokenCategory::SEPERATOR, ","},
                                    {TokenCategory::IMMEDIATE, std::to_string(upperBytes)}};

                LineTokens secondLine = tokenLine;
                ++it; // Increment iterator since we have added an instruction

                secondLine.tokens = {{TokenCategory::INSTRUCTION, "ori"},
                                     args[0],
                                     {TokenCategory::SEPERATOR, ","},
                                     {TokenCategory::REGISTER, "at"},
                                     {TokenCategory::SEPERATOR, ","},
                                     {TokenCategory::IMMEDIATE, std::to_string(lowerBytes)}};
                it = tokens.insert(it, secondLine);
            }
            // move $tx, $ty -> addu $tx, $ty, $zero
            else if (instructionName == "move") {
                tokenLine.tokens = {
                        {TokenCategory::INSTRUCTION, "addu"}, args[0],
                        {TokenCategory::SEPERATOR, ","},      {TokenCategory::REGISTER, "zero"},
                        {TokenCategory::SEPERATOR, ","},      args[1]};
            }
            // mul $tx, $ty, $tz -> mult $ty, $tz; mflo $tx
            else if (instructionName == "mul") {
                tokenLine.tokens = {{TokenCategory::INSTRUCTION, "mult"},
                                    args[1],
                                    {TokenCategory::SEPERATOR, ","},
                                    args[2]};

                LineTokens secondLine = tokenLine;
                ++it; // Increment iterator since we have added an instruction

                secondLine.tokens = {{TokenCategory::INSTRUCTION, "mflo"},
                                     {TokenCategory::REGISTER, args[0].value}};
                tokens.insert(it, secondLine);
            }
            // subi $tx, $ty, imm  -> addi $tx, $ty, -imm
            else if (instructionName == "subi") {
                uint32_t immediate = -stoui32(args[2].value);
                tokenLine.tokens = {{TokenCategory::INSTRUCTION, "addi"},
                                    args[0],
                                    {TokenCategory::SEPERATOR, ","},
                                    args[1],
                                    {TokenCategory::SEPERATOR, ","},
                                    {TokenCategory::IMMEDIATE, std::to_string(immediate)}};
            }
            // nop -> sll $zero, $zero, 0
            else if (instructionName == "nop") {
                tokenLine.tokens = {
                        {TokenCategory::INSTRUCTION, "sll"}, {TokenCategory::REGISTER, "zero"},
                        {TokenCategory::SEPERATOR, ","},     {TokenCategory::REGISTER, "zero"},
                        {TokenCategory::SEPERATOR, ","},     {TokenCategory::IMMEDIATE, "0"}};
            }
            // beqz $t0, label -> beq $t0, $zero, label
            else if (instructionName == "beqz") {
                tokenLine.tokens = {
                        {TokenCategory::INSTRUCTION, "beq"}, args[0],
                        {TokenCategory::SEPERATOR, ","},     {TokenCategory::REGISTER, "zero"},
                        {TokenCategory::SEPERATOR, ","},     args[1]};
            }
            // bnez $t0, label -> bne $t0, $zero, label
            else if (instructionName == "bnez") {
                tokenLine.tokens = {
                        {TokenCategory::INSTRUCTION, "bne"}, args[0],
                        {TokenCategory::SEPERATOR, ","},     {TokenCategory::REGISTER, "zero"},
                        {TokenCategory::SEPERATOR, ","},     args[1]};
            }

            std::vector<std::vector<Token>> branchLines;
            // bxx $tx, $tx, label -> slt $at, $tx, $tx; bxx $at, $zero, label
            std::vector<std::string> branchPseudoInstrs = {"blt", "bgt", "ble", "bge"};
            if (std::ranges::find(branchPseudoInstrs, instructionName) !=
                branchPseudoInstrs.end()) {
                if (instructionName == branchPseudoInstrs[0])
                    branchLines =
                            parseBranchPseudoInstruction(args[0], args[1], args[2], true, false);
                if (instructionName == branchPseudoInstrs[1])
                    branchLines =
                            parseBranchPseudoInstruction(args[0], args[1], args[2], false, false);
                if (instructionName == branchPseudoInstrs[2])
                    branchLines =
                            parseBranchPseudoInstruction(args[0], args[1], args[2], false, true);
                if (instructionName == branchPseudoInstrs[3])
                    branchLines =
                            parseBranchPseudoInstruction(args[0], args[1], args[2], true, true);
            }

            // bxxz $tx, label -> slt $at, $zero, $tx; bxx $at, $zero, label
            const Token regZero = {TokenCategory::REGISTER, "zero"};
            std::vector<std::string> branchZeroPseudoInstrs = {"bltz", "bgtz", "blez", "bgez"};
            if (std::ranges::find(branchZeroPseudoInstrs, instructionName) !=
                branchZeroPseudoInstrs.end()) {
                if (instructionName == branchZeroPseudoInstrs[0])
                    branchLines =
                            parseBranchPseudoInstruction(args[0], regZero, args[1], true, false);
                if (instructionName == branchZeroPseudoInstrs[1])
                    branchLines =
                            parseBranchPseudoInstruction(args[0], regZero, args[1], false, false);
                if (instructionName == branchZeroPseudoInstrs[2])
                    branchLines =
                            parseBranchPseudoInstruction(args[0], regZero, args[1], false, true);
                if (instructionName == branchZeroPseudoInstrs[3])
                    branchLines =
                            parseBranchPseudoInstruction(args[0], regZero, args[1], true, true);
            }

            if (!branchLines.empty()) {
                // Insert the parsed branch pseudo instruction lines into the token list
                for (const std::vector<Token>& branchLine : branchLines) {
                    LineTokens line = originalTokenLine;
                    line.tokens = branchLine;
                    it = tokens.insert(it + 1, line);
                }
                tokens.erase(it - 2); // Remove the pseudo instruction line
            } else
                ++it;
        } catch (const std::runtime_error& e) {
            throw MasmSyntaxError(e.what(), originalTokenLine.filename, originalTokenLine.lineno);
        }
    }
}


std::vector<std::vector<Token>>
Parser::parseBranchPseudoInstruction(const Token& reg1, const Token& reg2, const Token& label,
                                     const bool checkLt, const bool checkEq) {
    std::vector<std::vector<Token>> branchTokens = {{}, {}};
    // Swap argument order when checking for less than or greater than relationship
    if (checkLt)
        branchTokens[0] = {{TokenCategory::INSTRUCTION, "slt"}, {TokenCategory::REGISTER, "at"},
                           {TokenCategory::SEPERATOR, ","},     reg1,
                           {TokenCategory::SEPERATOR, ","},     reg2};
    else
        branchTokens[0] = {{TokenCategory::INSTRUCTION, "slt"}, {TokenCategory::REGISTER, "at"},
                           {TokenCategory::SEPERATOR, ","},     reg2,
                           {TokenCategory::SEPERATOR, ","},     reg1};

    if (checkEq)
        branchTokens[1] = {{TokenCategory::INSTRUCTION, "beq"}, {TokenCategory::REGISTER, "at"},
                           {TokenCategory::SEPERATOR, ","},     {TokenCategory::REGISTER, "zero"},
                           {TokenCategory::SEPERATOR, ","},     label};
    else
        branchTokens[1] = {{TokenCategory::INSTRUCTION, "bne"}, {TokenCategory::REGISTER, "at"},
                           {TokenCategory::SEPERATOR, ","},     {TokenCategory::REGISTER, "zero"},
                           {TokenCategory::SEPERATOR, ","},     label};

    return branchTokens;
}
