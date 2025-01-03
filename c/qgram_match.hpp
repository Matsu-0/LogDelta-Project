#ifndef QGRAM_MATCH_HPP
#define QGRAM_MATCH_HPP

#include <string>
#include <vector>
#include <utility>

// Operation item structure for storing match operations
struct OperationItem {
    int position;
    int length1;
    int length2;
    std::string substr;
    
    OperationItem(int pos, int len1, int len2, const std::string& sub);
};

// Generate q-grams from input string
std::vector<std::string> getQgram(const std::string& str, int k = 3);

// Get Q-gram match operations and distance
std::pair<std::vector<OperationItem>, double> getQgramMatchOplist(
    const std::string& str1, 
    const std::string& str2, 
    int k = 3
);

// Recover string using operation list
std::string recoverString(
    const std::vector<OperationItem>& operationList, 
    const std::string& str1
);

#endif // QGRAM_MATCH_HPP
