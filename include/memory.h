//
// Created by matthew on 4/16/25.
//

#ifndef MEMORY_H
#define MEMORY_H
#include <cstdint>
#include <string>


enum class MemSection { DATA, TEXT };


MemSection nameToMemSection(const std::string& name);


uint32_t memSectionOffset(MemSection section);


#endif // MEMORY_H
