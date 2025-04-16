//
// Created by matthew on 4/15/25.
//

#include "utils.h"
#include <regex>

bool isSignedInteger(const std::string& str) {
    const std::regex pattern("^[-]?[0-9]+$");
    return std::regex_match(str, pattern);
}
