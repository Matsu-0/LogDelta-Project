#include "variable_length_substitution.hpp"
#include "qgram_match.hpp"
#include <string>
#include <vector>
#include <utility>

std::pair<std::vector<OperationItem>, double> getSubstitutionOplist(
    // TODO: Implement variable length substitution algorithm
    const std::string& str1, const std::string& str2) {
    
    std::vector<OperationItem> operationList;
    double distance = 0;

    // If strings are identical, return empty operation list and zero distance
    if (str1 == str2) {
        return {operationList, distance};
    }

    // Create an operation item representing the entire string substitution
    operationList.emplace_back(0, str1.length(), str2.length(), str2);
    
    // Calculate distance: base cost (5.0) plus operation cost (3.0 + substitution length)
    distance = 5.0 + 3.0 + str2.length();

    return {operationList, distance};
} 