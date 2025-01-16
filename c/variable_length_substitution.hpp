#ifndef VARIABLE_LENGTH_SUBSTITUTION_HPP
#define VARIABLE_LENGTH_SUBSTITUTION_HPP

#include "qgram_match.hpp"
#include <string>
#include <vector>
#include <utility>

// Get substitution operations and distance for variable length strings
std::pair<std::vector<OperationItem>, double> getSubstitutionOplist(
    const std::string& str1,
    const std::string& str2
);

#endif // VARIABLE_LENGTH_SUBSTITUTION_HPP 