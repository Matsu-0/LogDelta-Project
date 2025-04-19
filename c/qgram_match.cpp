#include "qgram_match.hpp"
#include <unordered_map>
#include <algorithm>
#include <iostream>

OperationItem::OperationItem(int pos, int len1, int len2, const std::string& sub)
    : position(pos), length1(len1), length2(len2), substr(sub) {}

// Generate Q-grams with padding
std::vector<std::string> getQgram(const std::string& str, int k) {
    std::vector<std::string> qgramList;
    // Add padding characters to handle string boundaries
    std::string paddedStr = std::string(k-1, '$') + str + std::string(k-1, '#');
    
    // Generate q-grams using sliding window
    for (size_t i = 0; i <= paddedStr.length() - k; ++i) {
        qgramList.push_back(paddedStr.substr(i, k));
    }
    return qgramList;
}

std::pair<std::vector<OperationItem>, double> getQgramMatchOplist(
    const std::string& str1, const std::string& str2, int k) {
    
    int lenStr1 = str1.length();
    int lenStr2 = str2.length();

    // Generate Q-grams
    auto qGram1 = getQgram(str1, k);
    auto qGram2 = getQgram(str2, k);

    // Count Q-grams
    std::unordered_map<std::string, int> q1Counter, q2Counter;
    for (const auto& qgram : qGram1) q1Counter[qgram]++;
    for (const auto& qgram : qGram2) q2Counter[qgram]++;

    // Find common Q-grams
    std::vector<std::string> commonQGram;
    for (const auto& pair : q1Counter) {
        if (q2Counter.count(pair.first) > 0) {
            commonQGram.push_back(pair.first);
        }
    }
    std::sort(commonQGram.begin(), commonQGram.end());

    // Create Q-gram dictionary
    std::unordered_map<std::string, int> qDict;
    for (size_t i = 0; i < commonQGram.size(); ++i) {
        qDict[commonQGram[i]] = i;
    }

    // Map Q-grams to common indices
    std::vector<int> q1ToCommonMap;
    std::vector<int> q1Common;
    std::vector<int> q1CommonIndex;
    
    for (size_t i = 0; i < qGram1.size(); ++i) {
        const auto& item = qGram1[i];
        int idx = qDict.count(item) ? qDict[item] : -1;
        q1ToCommonMap.push_back(idx);
        if (idx != -1) {
            q1Common.push_back(idx);
            q1CommonIndex.push_back(i);
        }
    }

    std::vector<int> q2ToCommonMap;
    std::vector<int> q2Common;
    std::vector<int> q2CommonIndex;
    
    for (size_t i = 0; i < qGram2.size(); ++i) {
        const auto& item = qGram2[i];
        int idx = qDict.count(item) ? qDict[item] : -1;
        q2ToCommonMap.push_back(idx);
        if (idx != -1) {
            q2Common.push_back(idx);
            q2CommonIndex.push_back(i);
        }
    }

    // Find matches in Q-grams
    std::vector<std::pair<int, int>> matchListInQGram;
    size_t index1 = 0, index2 = 0;
    
    while (index1 < q1Common.size()) {
        int qGramIndex = q1Common[index1];
        const std::string& qGram = commonQGram[qGramIndex];
        
        if (q2Counter[qGram] != 0) {
            while (index2 < q2Common.size() && q2Common[index2] != qGramIndex) {
                q2Counter[commonQGram[q2Common[index2]]]--;
                index2++;
            }
            if (index2 < q2Common.size() && q2Common[index2] == qGramIndex) {
                matchListInQGram.push_back({q1CommonIndex[index1], q2CommonIndex[index2]});
                q2Counter[qGram]--;
                index2++;
            }
        }
        index1++;
    }

    // Merge matches
    std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>> mergeMatchList;
    std::reverse(matchListInQGram.begin(), matchListInQGram.end());
    
    if (!matchListInQGram.empty()) {
        auto preItem = matchListInQGram[0];
        auto beginItem = preItem;
        
        for (size_t i = 1; i < matchListInQGram.size(); ++i) {
            const auto& item = matchListInQGram[i];
            // Continue merge
            if (preItem.first == item.first + 1 && preItem.second == item.second + 1) {
                preItem = item;
            }
            // No repeat (new item)
            else if (preItem.first > item.first + (k-1) && preItem.second > item.second + (k-1)) {
                mergeMatchList.push_back({beginItem, preItem});
                preItem = item;
                beginItem = item;
            }
        }
        mergeMatchList.push_back({beginItem, preItem});
    }

    // Generate match string positions
    std::vector<std::vector<int>> matchStringPosition;
    for (const auto& matchItem : mergeMatchList) {
        int xBegin = matchItem.second.first - (k-1);
        int xEnd = matchItem.first.first;
        int yBegin = matchItem.second.second - (k-1);
        int yEnd = matchItem.first.second;

        xBegin = std::max(0, xBegin);
        yBegin = std::max(0, yBegin);
        xEnd = std::min(xEnd, lenStr1 - 1);
        yEnd = std::min(yEnd, lenStr2 - 1);
        
        matchStringPosition.push_back({xBegin, xEnd, yBegin, yEnd});
    }

    std::reverse(matchStringPosition.begin(), matchStringPosition.end());

    // Generate operation list
    std::vector<OperationItem> operationList;
    std::vector<int> preItem = {-1, -1, -1, -1};
    
    for (const auto& item : matchStringPosition) {
        if (item[0] != 0 || item[2] != 0) {
            int position = preItem[1] + 1;
            int length1 = item[0] - preItem[1] - 1;
            int length2 = item[2] - preItem[3] - 1;
            std::string substr = str2.substr(preItem[3] + 1, item[2] - preItem[3] - 1);
            operationList.emplace_back(position, length1, length2, substr);
        }
        preItem = item;
    }

    // Handle the last part
    if (lenStr1 != preItem[1] + 1 || lenStr2 != preItem[3] + 1) {
        operationList.emplace_back(
            preItem[1] + 1,
            lenStr1 - preItem[1] - 1,
            lenStr2 - preItem[3] - 1,
            str2.substr(preItem[3] + 1)
        );
    }

    // Compute distance
    double distance = 5.0;  // Base distance
    for (const auto& op : operationList) {
        distance += 3.0 + op.length2;
    }

    return {operationList, distance};
}

std::string recoverQgramString(
    const std::vector<OperationItem>& operationList,
    const std::string& str1) {
    std::string result;
    size_t oldPos = 0;
    
    for (const auto& op : operationList) {
        result += str1.substr(oldPos, op.position - oldPos);
        result += op.substr;
        oldPos = op.position + op.length1;
    }
    
    result += str1.substr(oldPos);
    return result;
}

// Comment out the test main function
/*
int main() {
    std::string str1 = "Jun 11 09:46:15 combo sshd: authentication failure; logname= uid=0 euid=0 tty=NODEVssh ruser= rhost=unknown.sagonet.net  user=root";
    std::string str2 = "Jun 11 09:46:18 combo sshd(pam_unix)[6488]: authentication failure; logname= uid=0 euid=0 tty=NODEVssh ruser= rhost=unknown.sagonet.net  user=rst";

    std::pair<std::vector<OperationItem>, double> result = getQgramMatchOplist(str1, str2, 3);
    const std::vector<OperationItem>& operationList = result.first;
    for (const auto& op : operationList) {
        std::cout << op.position << " " << op.length1 << " " << op.length2 << " " << op.substr << std::endl;
    }
    double distance = result.second;
    
    std::string recovered = recoverQgramString(operationList, str1);
    
    std::cout << "Distance: " << distance << std::endl;
    std::cout << "Original : " << str2 << std::endl;
    std::cout << "Recovered: " << recovered << std::endl;
}
*/