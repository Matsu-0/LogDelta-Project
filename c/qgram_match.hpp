#ifndef QGRAM_MATCH_HPP
#define QGRAM_MATCH_HPP

#include <string>
#include <vector>
#include <utility>

// Operation item structure for storing match operations
struct OperationItem {
    int position;      // Position in the original string
    int length1;       // Length of the substring to be replaced
    int length2;       // Length of the replacement substring
    std::string substr;// Replacement substring
    
    OperationItem(int pos, int len1, int len2, const std::string& sub);
};

// Generate q-grams from input string with specified length k
std::vector<std::string> getQgram(const std::string& str, int k = 3);

// Get Q-gram match operations and distance between two strings
std::pair<std::vector<OperationItem>, double> getQgramMatchOplist(
    const std::string& str1, 
    const std::string& str2, 
    int k = 3
);

// Recover string using Q-gram operation list
std::string recoverQgramString(
    const std::vector<OperationItem>& operationList, 
    const std::string& str1
);

#endif // QGRAM_MATCH_HPP
