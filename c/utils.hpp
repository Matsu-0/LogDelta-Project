// utils.h
#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>
#include <map>

// Convert binary string to bytes
std::vector<unsigned char> stringToBytes(const std::string& binaryString);

// Convert bytes to binary string
std::string bytesToString(const std::vector<unsigned char>& bytes);

// 目录操作相关函数
bool create_directory(const std::string& path);
bool directory_exists(const std::string& path);
bool ensure_directory_exists(const std::string& path);

// CSV文件写入相关函数
bool write_csv(const std::string& filepath,
              const std::map<std::string, std::vector<double>>& time_sets,
              const std::vector<std::string>& column_names,
              const std::vector<std::string>& first_column_values);

#endif // UTILS_HPP