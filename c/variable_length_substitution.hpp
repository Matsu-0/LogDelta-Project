#ifndef VARIABLE_LENGTH_SUBSTITUTION_HPP
#define VARIABLE_LENGTH_SUBSTITUTION_HPP

#include <string>
#include <vector>
#include <tuple>
#include "qgram_match.hpp"

// Cost weights for variable length substitution operations
namespace SubstitutionCost {
    const double POSITION_COST = 1.0;  // Cost for each position
    const double LENGTH_COST = 1.0;    // Cost for each length
    const double CHAR_COST = 1.0;      // Cost for each character
}

// Define a struct for position pair
struct Position {
    int i, j;
    Position(int i_, int j_) : i(i_), j(j_) {}
};

// Function to get search range positions
std::vector<Position> getSearchRange(int i, int j);

// Get substitution operations and distance for variable length strings
std::pair<std::vector<OperationItem>, double> getSubstitutionOplist(
    const std::string& str1,
    const std::string& str2
);

// Recover string using substitution operation list
std::string recoverSubstitutionString(
    const std::vector<OperationItem>& operationList,
    const std::string& str1
);

#endif // VARIABLE_LENGTH_SUBSTITUTION_HPP 