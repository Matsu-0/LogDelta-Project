#include "variable_length_substitution.hpp"
#include "qgram_match.hpp"
#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <iostream>  // For testing
#include <limits>
#include <tuple>
#include <iomanip>   // For std::setprecision
#include <fstream>   // For file operations

std::vector<Position> getSearchRange(int x, int y, const std::string& str1, const std::string& str2, int lmax = 3) {
    std::vector<Position> K;  // Store feasible position tuples
    
    // Check boundary conditions
    if (x <= 0 || y <= 0) {
        return K;
    }
    
    // Create counting array C
    std::vector<std::vector<int>> C(x + 1, std::vector<int>(y + 1, 0));
    
    // Fill C array from current position backwards with pruning
    for (int i = x - 1; i >= 0; i--) {
        bool row_pruned = false;  // Flag to track if current row should be pruned
        
        for (int j = y - 1; j >= 0 && !row_pruned; j--) {
            if (str1[i] == str2[j]) {
                C[i][j] = C[i + 1][j + 1] + 1;
            } else {
                C[i][j] = std::max(C[i + 1][j], C[i][j + 1]);
            }
            
            // Check if we should prune this direction
            if (C[i][j] >= lmax) {
                row_pruned = true;  // Stop processing this row
                continue;
            }
            
            // Add position if not at current position
            if (!(i == x && j == y)) {
                K.emplace_back(i, j);
            }
        }
        
        // If this row was pruned, we can stop processing further rows
        if (row_pruned) break;
    }
    
    return K;
}

std::pair<std::vector<OperationItem>, double> getSubstitutionOplist(
    const std::string& str1, const std::string& str2) {
    
    std::vector<OperationItem> operationList;
    double distance = 0;

    // If strings are identical, return empty operation list and zero distance
    if (str1 == str2) {
        return {operationList, distance};
    }

    int m = str1.length();
    int n = str2.length();

    // Handle empty string cases directly
    if (m == 0) {
        // Empty source string - single insertion of entire target
        operationList.emplace_back(0, 0, n, str2);
        return {operationList, SubstitutionCost::POSITION_COST + 
                             2 * SubstitutionCost::LENGTH_COST * 2 + 
                             SubstitutionCost::CHAR_COST * n};
    }
    if (n == 0) {
        // Empty target string - single deletion of entire source
        operationList.emplace_back(0, m, 0, "");
        return {operationList, SubstitutionCost::POSITION_COST + 
                             2 * SubstitutionCost::LENGTH_COST * 2};
    }

    // Create DP table for edit distance calculation
    std::vector<std::vector<double>> dp(m + 1, std::vector<double>(n + 1));
    // Create backtrack table that stores both operation type and best previous position
    std::vector<std::vector<std::pair<char, Position>>> backtrack(
        m + 1, std::vector<std::pair<char, Position>>(n + 1, {'M', Position(-1, -1)}));

    // Initialize first row and column
    for (int j = 0; j <= n; j++) {
        if (j == 0) {
            dp[0][j] = 0;
            backtrack[0][j] = {'M', Position(-1, -1)};  // Keep initial position as M
        } else {
            // All insertions from empty string need full costs
            dp[0][j] = SubstitutionCost::POSITION_COST + 
                      SubstitutionCost::LENGTH_COST * 2 + 
                      SubstitutionCost::CHAR_COST * j;
            backtrack[0][j] = {'X', Position(0, j-1)};  // Transfer from previous position
        }
    }

    // First column: converting source string to empty string
    for (int i = 0; i <= m; i++) {
        if (i == 0) {
            dp[i][0] = 0;
            backtrack[i][0] = {'M', Position(-1, -1)};  // Keep initial position as M
        } else {
            // All deletions to empty string need full costs
            dp[i][0] = SubstitutionCost::POSITION_COST + 
                      SubstitutionCost::LENGTH_COST * 2;
            backtrack[i][0] = {'X', Position(i-1, 0)};  // Transfer from previous position
        }
    }

    // Fill the DP table with weighted costs
    for (int i = 1; i <= m; i++) {
        for (int j = 1; j <= n; j++) {
            if (str1[i-1] == str2[j-1]) {
                dp[i][j] = dp[i-1][j-1];
                backtrack[i][j] = {'M', Position(i-1, j-1)};
            } else {
                auto range = getSearchRange(i, j, str1, str2);
                double min_cost = std::numeric_limits<double>::max();
                Position best_prev(-1, -1);

                // For each possible previous position
                for (const auto& pos : range) {
                    if (pos.i >= 0 && pos.j >= 0) {
                        double cost = dp[pos.i][pos.j];
                        
                        // All operations are treated as substitutions
                        if (backtrack[pos.i][pos.j].first == 'M') {
                            // New substitution operation
                            cost += SubstitutionCost::POSITION_COST + 
                                   SubstitutionCost::LENGTH_COST * 2;
                        }
                        // Add character cost for the new characters
                        cost += SubstitutionCost::CHAR_COST * (j - pos.j);

                        if (cost < min_cost) {
                            min_cost = cost;
                            dp[i][j] = cost;
                            best_prev = pos;
                        }
                    }
                }

                backtrack[i][j] = {'X', best_prev};
            }
        }
    }

    // Reconstruct the operations by backtracking
    int i = m, j = n;
    std::vector<std::tuple<int, int, int, std::string>> temp_ops;
    
    while (i > 0 || j > 0) {
        auto [op_type, prev_pos] = backtrack[i][j];
        
        if (op_type == 'M') {
            i = prev_pos.i;
            j = prev_pos.j;
            continue;
        }

        // Create substitution operation using stored previous position
        int del_len = i - prev_pos.i;
        int ins_len = j - prev_pos.j;
        std::string new_text = str2.substr(prev_pos.j, ins_len);
        
        temp_ops.emplace_back(prev_pos.i, del_len, ins_len, new_text);
        
        i = prev_pos.i;
        j = prev_pos.j;
    }

    // Process and merge operations from left to right
    for (auto it = temp_ops.rbegin(); it != temp_ops.rend(); ++it) {
        auto [pos, del_len, ins_len, substr] = *it;
        
        // Try to merge with previous operation if possible
        if (!operationList.empty()) {
            auto& last_op = operationList.back();
            // Check if operations are adjacent in the original string
            if (last_op.position + last_op.length1 == pos) {
                // Merge operations - all operations are treated as substitutions
                last_op.length1 += del_len;
                if (ins_len > 0) {
                    last_op.length2 += ins_len;
                    last_op.substr += substr;
                }
                continue;
            }
        }
        
        // Create new operation
        operationList.emplace_back(pos, del_len, ins_len, substr);
    }

    // Calculate final distance using new weighting system
    distance = dp[m][n];  // Use the final DP table value as distance

    return {operationList, distance};
} 

std::string recoverSubstitutionString(
    const std::vector<OperationItem>& operationList,
    const std::string& str1) {
    
    std::string result;
    size_t oldPos = 0;
    
    for (const auto& op : operationList) {
        // Copy unchanged part
        result += str1.substr(oldPos, op.position - oldPos);
        
        if (op.length2 > 0) {  // Insertion or substitution
            result += op.substr;
        }
        // Skip deleted part in original string
        oldPos = op.position + op.length1;
    }
    
    // Append remaining part of original string
    result += str1.substr(oldPos);
    
    return result;
}

// 辅助函数：打印 DP 表格为 CSV 格式并保存到文件
void printDPTableCSV(const std::vector<std::vector<double>>& dp, 
                     const std::string& str1, const std::string& str2) {
    std::cout << "\n=== DP 表格 (CSV 格式) ===\n";
    
    // 创建 CSV 文件
    std::ofstream csvFile("dp_table.csv");
    if (!csvFile.is_open()) {
        std::cerr << "无法创建 CSV 文件！" << std::endl;
        return;
    }
    
    // 输出表头（列标签）
    std::cout << ",";
    csvFile << ",";
    for (size_t j = 0; j <= str2.length(); j++) {
        if (j == 0) {
            std::cout << "ε";
            csvFile << "ε";
        } else {
            std::cout << str2[j-1];
            csvFile << str2[j-1];
        }
        if (j < str2.length()) {
            std::cout << ",";
            csvFile << ",";
        }
    }
    std::cout << "\n";
    csvFile << "\n";
    
    // 输出数据行
    for (size_t i = 0; i <= str1.length(); i++) {
        // 行标签
        if (i == 0) {
            std::cout << "ε";
            csvFile << "ε";
        } else {
            std::cout << str1[i-1];
            csvFile << str1[i-1];
        }
        
        // 数据列
        for (size_t j = 0; j <= str2.length(); j++) {
            std::cout << "," << std::fixed << std::setprecision(2) << dp[i][j];
            csvFile << "," << std::fixed << std::setprecision(2) << dp[i][j];
        }
        std::cout << "\n";
        csvFile << "\n";
    }
    std::cout << "\n";
    csvFile.close();
    
    std::cout << "DP 表格已保存到 dp_table.csv 文件中\n";
}

// 辅助函数：打印 SearchRange 结果并保存到文件
void printSearchRange(const std::vector<Position>& range, int x, int y, std::ofstream& csvFile) {
    std::cout << "位置 (" << x << ", " << y << ") 的 SearchRange:\n";
    csvFile << "位置 (" << x << ", " << y << ") 的 SearchRange:\n";
    
    if (range.empty()) {
        std::cout << "  无可行位置\n";
        csvFile << "  无可行位置\n";
    } else {
        std::cout << "  可行位置数量: " << range.size() << "\n";
        csvFile << "  可行位置数量: " << range.size() << "\n";
        std::cout << "  位置列表: ";
        csvFile << "  位置列表: ";
        for (size_t i = 0; i < range.size(); i++) {
            std::cout << "(" << range[i].i << ", " << range[i].j << ")";
            csvFile << "(" << range[i].i << ", " << range[i].j << ")";
            if (i < range.size() - 1) {
                std::cout << ", ";
                csvFile << ", ";
            }
        }
        std::cout << "\n";
        csvFile << "\n";
    }
    std::cout << "\n";
    csvFile << "\n";
}
