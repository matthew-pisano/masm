//
// Created by matthew on 4/15/25.
//

#ifndef REGISTER_H
#define REGISTER_H

#include <array>
#include <cstdint>
#include <map>
#include <string>

class RegisterFile {
    std::array<uint32_t, 32> registers = {};

    std::map<std::string, int> nameToIndex = {
            {"zero", 0}, {"at", 1},  {"v0", 2},  {"v1", 3},  {"a0", 4},  {"a1", 5},  {"a2", 6},
            {"a3", 7},   {"t0", 8},  {"t1", 9},  {"t2", 10}, {"t3", 11}, {"t4", 12}, {"t5", 13},
            {"t6", 14},  {"t7", 15}, {"s0", 16}, {"s1", 17}, {"s2", 18}, {"s3", 19}, {"s4", 20},
            {"s5", 21},  {"s6", 22}, {"s7", 23}, {"t8", 24}, {"t9", 25}, {"k0", 26}, {"k1", 27},
            {"gp", 28},  {"sp", 29}, {"fp", 30}, {"ra", 31}};

public:
    int indexFromName(const std::string& name);

    uint32_t operator[](int index) const;
};

#endif // REGISTER_H
